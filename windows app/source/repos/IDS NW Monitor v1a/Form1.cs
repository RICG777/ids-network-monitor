using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Text.RegularExpressions;

namespace IDS_NW_Monitor_v1a
{
    public partial class Form1 : Form
    {
        private SerialPort serialPort;
        private const string BUILD_VERSION = "0.5.0";
        private const string EXPECTED_FW_VERSION = "1d.1";
        private bool isConnected = false;
        private StreamWriter logWriter = null;
        private string logFilePath = null;

        public Form1()
        {
            InitializeComponent();

            // Set application icon
            try
            {
                string iconPath = Path.Combine(Application.StartupPath, "app.ico");
                if (File.Exists(iconPath))
                    this.Icon = new Icon(iconPath);
            }
            catch { }

            // Populate COM ports
            RefreshCOMPorts();

            // Restore saved settings
            var settings = Properties.Settings.Default;
            if (!string.IsNullOrEmpty(settings.COMPort) && cmbCOMPorts.Items.Contains(settings.COMPort))
                cmbCOMPorts.SelectedItem = settings.COMPort;
            if (!string.IsNullOrEmpty(settings.IPAddress))
                txtIPAddress.Text = settings.IPAddress;
            if (!string.IsNullOrEmpty(settings.Trigger1))
                textBoxTrigger.Text = settings.Trigger1;
            if (!string.IsNullOrEmpty(settings.Trigger2))
                textBoxTrigger2.Text = settings.Trigger2;

            // Initialize serial port
            serialPort = new SerialPort();
            serialPort.BaudRate = 9600;
            serialPort.DataReceived += serialPort_DataReceived;

            // Update status bar
            statusFirmware.Text = "App v" + BUILD_VERSION;

            // Start log file
            StartLogFile();

            // Set initial UI state
            SetConnectedState(false);
        }

        // =====================
        // UI STATE MANAGEMENT
        // =====================

        private void SetConnectedState(bool connected)
        {
            isConnected = connected;

            grpNetwork.Enabled = connected;
            grpRelay1.Enabled = connected;
            grpRelay2.Enabled = connected;

            btnConnect.Text = connected ? "Disconnect" : "Connect";

            if (connected)
            {
                statusConnection.Text = " Connected";
                statusConnection.BackColor = Color.FromArgb(0, 140, 0);
                statusCOMPort.Text = serialPort.PortName;
            }
            else
            {
                statusConnection.Text = " Disconnected";
                statusConnection.BackColor = Color.FromArgb(180, 0, 0);
                statusCOMPort.Text = "";
                statusBoardIP.Text = "";
                SetRelayIndicator(1, false);
                SetRelayIndicator(2, false);
            }
        }

        private void SetRelayIndicator(int relay, bool on)
        {
            if (relay == 1)
            {
                pnlRelay1Indicator.BackColor = on ? Color.FromArgb(220, 40, 40) : Color.FromArgb(100, 100, 100);
                lblRelay1Status.Text = on ? "ENERGISED" : "OFF";
                lblRelay1Status.ForeColor = on ? Color.FromArgb(220, 40, 40) : Color.FromArgb(100, 100, 100);
            }
            else
            {
                pnlRelay2Indicator.BackColor = on ? Color.FromArgb(220, 40, 40) : Color.FromArgb(100, 100, 100);
                lblRelay2Status.Text = on ? "ENERGISED" : "OFF";
                lblRelay2Status.ForeColor = on ? Color.FromArgb(220, 40, 40) : Color.FromArgb(100, 100, 100);
            }
        }

        // =====================
        // LOGGING
        // =====================

        private void StartLogFile()
        {
            try
            {
                string logDir = Path.Combine(Application.StartupPath, "logs");
                Directory.CreateDirectory(logDir);
                logFilePath = Path.Combine(logDir, "ids_monitor_" + DateTime.Now.ToString("yyyy-MM-dd_HHmmss") + ".log");
                logWriter = new StreamWriter(logFilePath, true);
                logWriter.AutoFlush = true;
                logWriter.WriteLine("=== IDS Network Monitor v" + BUILD_VERSION + " started " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + " ===");
            }
            catch { }
        }

        private void LogDebug(string message)
        {
            string timestamped = "[" + DateTime.Now.ToString("HH:mm:ss") + "] " + message;

            // Write to file
            try { logWriter?.WriteLine(timestamped); } catch { }

            // Write to UI
            string uiLine = timestamped + "\r\n";
            if (txtDebug.InvokeRequired)
            {
                txtDebug.Invoke(new Action(() => { txtDebug.AppendText(uiLine); }));
            }
            else
            {
                txtDebug.AppendText(uiLine);
            }
        }

        private void btnClearLog_Click(object sender, EventArgs e)
        {
            txtDebug.Clear();
        }

        // =====================
        // CONNECTION
        // =====================

        private void RefreshCOMPorts()
        {
            string currentSelection = cmbCOMPorts.SelectedItem?.ToString();
            cmbCOMPorts.Items.Clear();
            cmbCOMPorts.Items.AddRange(SerialPort.GetPortNames());

            if (!string.IsNullOrEmpty(currentSelection) && cmbCOMPorts.Items.Contains(currentSelection))
                cmbCOMPorts.SelectedItem = currentSelection;
            else if (cmbCOMPorts.Items.Count > 0)
                cmbCOMPorts.SelectedIndex = 0;
        }

        private void btnRefreshPorts_Click(object sender, EventArgs e)
        {
            RefreshCOMPorts();
            LogDebug("COM ports refreshed. Found " + cmbCOMPorts.Items.Count + " port(s).");
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (!isConnected)
            {
                if (cmbCOMPorts.SelectedItem == null)
                {
                    LogDebug("No COM port selected.");
                    return;
                }

                serialPort.PortName = cmbCOMPorts.SelectedItem.ToString();

                try
                {
                    LogDebug("Opening " + serialPort.PortName + "...");
                    serialPort.Open();
                    serialPort.DtrEnable = true;
                    serialPort.RtsEnable = true;

                    SetConnectedState(true);

                    Properties.Settings.Default.COMPort = serialPort.PortName;
                    Properties.Settings.Default.Save();

                    LogDebug("Requesting board status...");
                    serialPort.Write("STATUS\n");
                }
                catch (Exception ex)
                {
                    LogDebug("Connection failed: " + ex.Message);
                    SetConnectedState(false);
                }
            }
            else
            {
                try
                {
                    LogDebug("Closing " + serialPort.PortName + "...");
                    serialPort.Close();
                }
                catch { }
                SetConnectedState(false);
            }
        }

        // =====================
        // VALIDATION
        // =====================

        private bool IsValidIPAddress(string ip)
        {
            string pattern = @"^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
            return Regex.IsMatch(ip, pattern);
        }

        private bool ValidateTrigger(string trigger, Label errorLabel)
        {
            if (string.IsNullOrWhiteSpace(trigger))
            {
                errorLabel.Text = "Trigger message cannot be empty";
                return false;
            }
            if (trigger.Length >= 80)
            {
                errorLabel.Text = "Must be less than 80 characters";
                return false;
            }
            errorLabel.Text = "";
            return true;
        }

        // =====================
        // NETWORK CONFIGURATION
        // =====================

        private void btnSetIP_Click(object sender, EventArgs e)
        {
            if (!IsValidIPAddress(txtIPAddress.Text))
            {
                lblIPError.Text = "Invalid IP address";
                return;
            }
            lblIPError.Text = "";

            var result = MessageBox.Show(
                "Apply network settings to the board?\n\nIP Address: " + txtIPAddress.Text,
                "Confirm Network Change",
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            if (result != DialogResult.Yes) return;

            serialPort.Write("IP:" + txtIPAddress.Text + "\n");
            LogDebug("Sent: IP:" + txtIPAddress.Text);

            Properties.Settings.Default.IPAddress = txtIPAddress.Text;
            Properties.Settings.Default.Save();
        }

        // =====================
        // RELAY CONFIGURATION
        // =====================

        private void btnSetTrigger_Click(object sender, EventArgs e)
        {
            string trigger = textBoxTrigger.Text.Trim();
            if (!ValidateTrigger(trigger, lblTrigger1Error)) return;

            var result = MessageBox.Show(
                "Set Relay 1 trigger to:\n\n\"" + trigger + "\"",
                "Confirm Trigger Change",
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            if (result != DialogResult.Yes) return;

            serialPort.Write("Relay1:" + trigger + "\n");
            LogDebug("Sent: Relay1:" + trigger);

            Properties.Settings.Default.Trigger1 = trigger;
            Properties.Settings.Default.Save();
        }

        private void btnSetTrigger2_Click(object sender, EventArgs e)
        {
            string trigger = textBoxTrigger2.Text.Trim();
            if (!ValidateTrigger(trigger, lblTrigger2Error)) return;

            var result = MessageBox.Show(
                "Set Relay 2 trigger to:\n\n\"" + trigger + "\"",
                "Confirm Trigger Change",
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            if (result != DialogResult.Yes) return;

            serialPort.Write("Relay2:" + trigger + "\n");
            LogDebug("Sent: Relay2:" + trigger);

            Properties.Settings.Default.Trigger2 = trigger;
            Properties.Settings.Default.Save();
        }

        // =====================
        // RELAY TESTING
        // =====================

        private void btnTestRelay1_Click(object sender, EventArgs e)
        {
            if (!isConnected) return;
            btnTestRelay1.Enabled = false;
            LogDebug("Testing Relay 1 (3 second pulse)...");
            serialPort.Write("TEST1\n");
        }

        private void btnTestRelay2_Click(object sender, EventArgs e)
        {
            if (!isConnected) return;
            btnTestRelay2.Enabled = false;
            LogDebug("Testing Relay 2 (3 second pulse)...");
            serialPort.Write("TEST2\n");
        }

        // =====================
        // ABOUT DIALOG
        // =====================

        private void btnAbout_Click(object sender, EventArgs e)
        {
            string aboutText =
                "IDS Network Monitor\n" +
                "Version " + BUILD_VERSION + "\n\n" +
                "Syslog-based relay trigger for network monitoring.\n" +
                "Monitors network switch events and activates physical\n" +
                "alarm relays when trigger conditions are detected.\n\n" +
                "Expected firmware: " + EXPECTED_FW_VERSION + "\n\n" +
                "\u00A9 " + DateTime.Now.Year + " Tier8 Ltd\n" +
                "www.tier8.uk";

            MessageBox.Show(aboutText, "About IDS Network Monitor",
                MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        // =====================
        // SERIAL DATA HANDLING
        // =====================

        private void serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                while (serialPort.BytesToRead > 0)
                {
                    string line = serialPort.ReadLine().Trim();
                    if (string.IsNullOrEmpty(line)) continue;

                    if (line.StartsWith("ArdSTATUS:FW:"))
                    {
                        string fw = line.Substring("ArdSTATUS:FW:".Length);
                        this.Invoke(new Action(() => {
                            statusFirmware.Text = "App v" + BUILD_VERSION + "  |  FW v" + fw;
                            if (fw != EXPECTED_FW_VERSION)
                            {
                                LogDebug("WARNING: Firmware version mismatch! Expected " + EXPECTED_FW_VERSION + ", got " + fw);
                                MessageBox.Show(
                                    "Firmware version mismatch.\n\n" +
                                    "Expected: " + EXPECTED_FW_VERSION + "\n" +
                                    "Found: " + fw + "\n\n" +
                                    "Some features may not work correctly.",
                                    "Firmware Mismatch",
                                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                            }
                        }));
                        LogDebug("Firmware version: " + fw);
                    }
                    else if (line.StartsWith("ArdSTATUS:IP:"))
                    {
                        string val = line.Substring("ArdSTATUS:IP:".Length);
                        this.Invoke(new Action(() => {
                            txtIPAddress.Text = val;
                            statusBoardIP.Text = "Board: " + val;
                        }));
                        LogDebug("Board IP: " + val);
                    }
                    else if (line.StartsWith("ArdSTATUS:Relay1:"))
                    {
                        string val = line.Substring("ArdSTATUS:Relay1:".Length);
                        this.Invoke(new Action(() => { textBoxTrigger.Text = val; }));
                        LogDebug("Relay 1 trigger: " + val);
                    }
                    else if (line.StartsWith("ArdSTATUS:Relay2:"))
                    {
                        string val = line.Substring("ArdSTATUS:Relay2:".Length);
                        this.Invoke(new Action(() => { textBoxTrigger2.Text = val; }));
                        LogDebug("Relay 2 trigger: " + val);
                    }
                    else if (line.StartsWith("ArdSTATUS:R1State:"))
                    {
                        bool on = line.Contains("ON");
                        this.Invoke(new Action(() => { SetRelayIndicator(1, on); }));
                        LogDebug("Relay 1 state: " + (on ? "ON" : "OFF"));
                    }
                    else if (line.StartsWith("ArdSTATUS:R2State:"))
                    {
                        bool on = line.Contains("ON");
                        this.Invoke(new Action(() => { SetRelayIndicator(2, on); }));
                        LogDebug("Relay 2 state: " + (on ? "ON" : "OFF"));
                    }
                    else if (line.StartsWith("ArdSTATUS:"))
                    {
                        LogDebug("Board: " + line.Substring("ArdSTATUS:".Length));
                    }
                    else if (line.StartsWith("ArdACK:TEST1:"))
                    {
                        this.Invoke(new Action(() => { SetRelayIndicator(1, true); }));
                        LogDebug("Relay 1 test pulse started.");
                    }
                    else if (line.StartsWith("ArdACK:TEST2:"))
                    {
                        this.Invoke(new Action(() => { SetRelayIndicator(2, true); }));
                        LogDebug("Relay 2 test pulse started.");
                    }
                    else if (line.Contains("test complete"))
                    {
                        if (line.Contains("Relay 1"))
                        {
                            this.Invoke(new Action(() => {
                                SetRelayIndicator(1, false);
                                btnTestRelay1.Enabled = true;
                            }));
                        }
                        else if (line.Contains("Relay 2"))
                        {
                            this.Invoke(new Action(() => {
                                SetRelayIndicator(2, false);
                                btnTestRelay2.Enabled = true;
                            }));
                        }
                        LogDebug(line);
                    }
                    else if (line.StartsWith("ArdACK:"))
                    {
                        LogDebug("Confirmed: " + line.Substring("ArdACK:".Length));
                    }
                    else if (line.StartsWith("ArdERR:"))
                    {
                        string err = line.Substring("ArdERR:".Length);
                        LogDebug("ERROR: " + err);
                        this.Invoke(new Action(() => {
                            MessageBox.Show("Board reported an error:\n\n" + err, "Board Error",
                                MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        }));
                    }
                    else if (line.Contains("Relay 1 ENERGIZED") || line.Contains("Relay 1 energized"))
                    {
                        this.Invoke(new Action(() => { SetRelayIndicator(1, true); }));
                        LogDebug("ALERT: " + line);
                    }
                    else if (line.Contains("Relay 2 ENERGIZED") || line.Contains("Relay 2 energized"))
                    {
                        this.Invoke(new Action(() => { SetRelayIndicator(2, true); }));
                        LogDebug("ALERT: " + line);
                    }
                    else if (line.Contains("Relay 1 de-energized"))
                    {
                        this.Invoke(new Action(() => { SetRelayIndicator(1, false); }));
                        LogDebug(line);
                    }
                    else if (line.Contains("Relay 2 de-energized"))
                    {
                        this.Invoke(new Action(() => { SetRelayIndicator(2, false); }));
                        LogDebug(line);
                    }
                    else
                    {
                        LogDebug("Board: " + line);
                    }
                }
            }
            catch (TimeoutException) { }
            catch (Exception ex)
            {
                LogDebug("Serial error: " + ex.Message);
            }
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            try
            {
                if (serialPort != null && serialPort.IsOpen)
                    serialPort.Close();
                if (logWriter != null)
                {
                    logWriter.WriteLine("=== Session ended " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + " ===");
                    logWriter.Close();
                }
            }
            catch { }
            base.OnFormClosing(e);
        }
    }
}
