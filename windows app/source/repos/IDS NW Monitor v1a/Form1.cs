using System;
using System.Drawing;
using System.IO;
using System.Windows.Forms;
using System.IO.Ports;
using System.Text.RegularExpressions;

namespace IDS_NW_Monitor_v1a
{
    public partial class Form1 : Form
    {
        private SerialPort serialPort;
        private const string BUILD_VERSION = "0.7.0";
        private const string EXPECTED_FW_VERSION = "1d.3";
        private bool isConnected = false;
        private bool suppressEvents = false;
        private StreamWriter logWriter = null;
        private static readonly int[] DurationValues = { 5, 10, 15, 30, 60, 120 };

        public Form1()
        {
            InitializeComponent();

            try
            {
                string iconPath = Path.Combine(Application.StartupPath, "app.ico");
                if (File.Exists(iconPath))
                    this.Icon = new Icon(iconPath);
            }
            catch { }

            RefreshCOMPorts();

            var s = Properties.Settings.Default;
            if (!string.IsNullOrEmpty(s.COMPort) && cmbCOMPorts.Items.Contains(s.COMPort))
                cmbCOMPorts.SelectedItem = s.COMPort;
            if (!string.IsNullOrEmpty(s.IPAddress))
                txtIPAddress.Text = s.IPAddress;
            if (!string.IsNullOrEmpty(s.Trigger1))
                txtTrig1A.Text = s.Trigger1;
            if (!string.IsNullOrEmpty(s.Trigger2))
                txtTrig2A.Text = s.Trigger2;

            serialPort = new SerialPort();
            serialPort.BaudRate = 9600;
            serialPort.DataReceived += serialPort_DataReceived;

            statusFirmware.Text = "App v" + BUILD_VERSION;
            StartLogFile();
            SetConnectedState(false);
        }

        // ===================== UI STATE =====================

        private void SetConnectedState(bool connected)
        {
            isConnected = connected;
            grpNetwork.Enabled = connected;
            grpBoardInfo.Enabled = connected;
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
                lblMACValue.Text = "\u2014";
                SetRelayIndicator(1, false);
                SetRelayIndicator(2, false);
            }
        }

        private void SetRelayIndicator(int relay, bool on)
        {
            var pnl = relay == 1 ? pnlRelay1Indicator : pnlRelay2Indicator;
            var lbl = relay == 1 ? lblRelay1Status : lblRelay2Status;
            pnl.BackColor = on ? Color.FromArgb(220, 40, 40) : Color.FromArgb(100, 100, 100);
            lbl.Text = on ? "ENERGISED" : "OFF";
            lbl.ForeColor = on ? Color.FromArgb(220, 40, 40) : Color.FromArgb(100, 100, 100);
        }

        // ===================== LOGGING =====================

        private void StartLogFile()
        {
            try
            {
                string logDir = Path.Combine(Application.StartupPath, "logs");
                Directory.CreateDirectory(logDir);
                logWriter = new StreamWriter(Path.Combine(logDir, "ids_monitor_" + DateTime.Now.ToString("yyyy-MM-dd_HHmmss") + ".log"), true);
                logWriter.AutoFlush = true;
                logWriter.WriteLine("=== IDS Network Monitor v" + BUILD_VERSION + " started " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + " ===");
            }
            catch { }
        }

        private void LogDebug(string message)
        {
            string ts = "[" + DateTime.Now.ToString("HH:mm:ss") + "] " + message;
            try { logWriter?.WriteLine(ts); } catch { }
            string uiLine = ts + "\r\n";
            if (txtDebug.InvokeRequired)
                txtDebug.Invoke(new Action(() => txtDebug.AppendText(uiLine)));
            else
                txtDebug.AppendText(uiLine);
        }

        private void btnClearLog_Click(object sender, EventArgs e) { txtDebug.Clear(); }

        // ===================== CONNECTION =====================

        private void RefreshCOMPorts()
        {
            string cur = cmbCOMPorts.SelectedItem?.ToString();
            cmbCOMPorts.Items.Clear();
            cmbCOMPorts.Items.AddRange(SerialPort.GetPortNames());
            if (!string.IsNullOrEmpty(cur) && cmbCOMPorts.Items.Contains(cur))
                cmbCOMPorts.SelectedItem = cur;
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
                if (cmbCOMPorts.SelectedItem == null) { LogDebug("No COM port selected."); return; }
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
                catch (Exception ex) { LogDebug("Connection failed: " + ex.Message); SetConnectedState(false); }
            }
            else
            {
                try { serialPort.Close(); } catch { }
                SetConnectedState(false);
            }
        }

        // ===================== VALIDATION =====================

        private bool IsValidIP(string ip)
        {
            return Regex.IsMatch(ip, @"^(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)$");
        }

        // ===================== NETWORK =====================

        private void btnSetNetwork_Click(object sender, EventArgs e)
        {
            if (!IsValidIP(txtIPAddress.Text)) { lblIPError.Text = "Invalid IP"; return; }
            lblIPError.Text = "";

            var r = MessageBox.Show("Apply network settings?\n\nIP: " + txtIPAddress.Text +
                "\nSubnet: " + txtSubnetMask.Text + "\nGateway: " + txtGateway.Text,
                "Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            if (r != DialogResult.Yes) return;

            serialPort.Write("IP:" + txtIPAddress.Text + "\n");
            if (IsValidIP(txtSubnetMask.Text))
                serialPort.Write("SUBNET:" + txtSubnetMask.Text + "\n");
            if (!string.IsNullOrWhiteSpace(txtGateway.Text) && IsValidIP(txtGateway.Text))
                serialPort.Write("GATEWAY:" + txtGateway.Text + "\n");
            LogDebug("Sent network config.");

            Properties.Settings.Default.IPAddress = txtIPAddress.Text;
            Properties.Settings.Default.Save();
        }

        // ===================== TRIGGER HELPERS =====================

        private void SendTrigger(int relay, char slot, TextBox txt)
        {
            string trig = txt.Text.Trim();
            if (trig.Length >= 40)
            {
                MessageBox.Show("Trigger must be less than 40 characters.", "Validation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            string cmd = "Relay" + relay + slot + ":" + trig + "\n";
            serialPort.Write(cmd);
            LogDebug("Sent: " + cmd.Trim());
        }

        private void btnSetTrig1A_Click(object sender, EventArgs e) { SendTrigger(1, 'A', txtTrig1A); }
        private void btnSetTrig1B_Click(object sender, EventArgs e) { SendTrigger(1, 'B', txtTrig1B); }
        private void btnSetTrig1C_Click(object sender, EventArgs e) { SendTrigger(1, 'C', txtTrig1C); }
        private void btnSetTrig2A_Click(object sender, EventArgs e) { SendTrigger(2, 'A', txtTrig2A); }
        private void btnSetTrig2B_Click(object sender, EventArgs e) { SendTrigger(2, 'B', txtTrig2B); }
        private void btnSetTrig2C_Click(object sender, EventArgs e) { SendTrigger(2, 'C', txtTrig2C); }

        // ===================== MODE & DURATION =====================

        private void cmbMode1_Changed(object sender, EventArgs e)
        {
            if (!isConnected || suppressEvents) return;
            serialPort.Write("MODE1:" + (cmbMode1.SelectedIndex == 1 ? "LATCH" : "PULSE") + "\n");
        }
        private void cmbMode2_Changed(object sender, EventArgs e)
        {
            if (!isConnected || suppressEvents) return;
            serialPort.Write("MODE2:" + (cmbMode2.SelectedIndex == 1 ? "LATCH" : "PULSE") + "\n");
        }
        private void cmbDuration1_Changed(object sender, EventArgs e)
        {
            if (!isConnected || suppressEvents) return;
            serialPort.Write("DURATION1:" + DurationValues[cmbDuration1.SelectedIndex] + "\n");
        }
        private void cmbDuration2_Changed(object sender, EventArgs e)
        {
            if (!isConnected || suppressEvents) return;
            serialPort.Write("DURATION2:" + DurationValues[cmbDuration2.SelectedIndex] + "\n");
        }
        private void cmbVerbosity_Changed(object sender, EventArgs e)
        {
            if (!isConnected || suppressEvents) return;
            serialPort.Write("VERBOSITY:" + cmbVerbosity.SelectedIndex + "\n");
            LogDebug("Sent: VERBOSITY:" + cmbVerbosity.SelectedIndex);
        }

        // ===================== RESET & TEST =====================

        private void btnResetRelay1_Click(object sender, EventArgs e) { if (isConnected) serialPort.Write("RESET1\n"); }
        private void btnResetRelay2_Click(object sender, EventArgs e) { if (isConnected) serialPort.Write("RESET2\n"); }

        private void btnTestRelay1_Click(object sender, EventArgs e)
        {
            if (!isConnected) return;
            btnTestRelay1.Enabled = false;
            serialPort.Write("TEST1\n");
        }
        private void btnTestRelay2_Click(object sender, EventArgs e)
        {
            if (!isConnected) return;
            btnTestRelay2.Enabled = false;
            serialPort.Write("TEST2\n");
        }
        private void btnTimedTestRelay1_Click(object sender, EventArgs e)
        {
            if (!isConnected) return;
            int dur = DurationValues[cmbDuration1.SelectedIndex];
            if (MessageBox.Show("Timed test: energize Relay 1 for " + dur + "s?", "Confirm", MessageBoxButtons.YesNo) != DialogResult.Yes) return;
            btnTimedTestRelay1.Enabled = false;
            serialPort.Write("TTEST1\n");
        }
        private void btnTimedTestRelay2_Click(object sender, EventArgs e)
        {
            if (!isConnected) return;
            int dur = DurationValues[cmbDuration2.SelectedIndex];
            if (MessageBox.Show("Timed test: energize Relay 2 for " + dur + "s?", "Confirm", MessageBoxButtons.YesNo) != DialogResult.Yes) return;
            btnTimedTestRelay2.Enabled = false;
            serialPort.Write("TTEST2\n");
        }

        // ===================== ABOUT =====================

        private void btnAbout_Click(object sender, EventArgs e)
        {
            MessageBox.Show(
                "IDS Network Monitor\nVersion " + BUILD_VERSION + "\n\n" +
                "Syslog-based relay trigger for network monitoring.\n\n" +
                "Expected firmware: " + EXPECTED_FW_VERSION + "\n\n" +
                "\u00A9 " + DateTime.Now.Year + " Tier8 Ltd\nwww.tier8.uk",
                "About", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        // ===================== DURATION HELPER =====================

        private void SelectDuration(ComboBox cmb, int seconds)
        {
            for (int i = 0; i < DurationValues.Length; i++)
                if (DurationValues[i] == seconds) { cmb.SelectedIndex = i; return; }
            cmb.SelectedIndex = 1;
        }

        // ===================== SERIAL DATA =====================

        private void serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                while (serialPort.BytesToRead > 0)
                {
                    string line = serialPort.ReadLine().Trim();
                    if (string.IsNullOrEmpty(line)) continue;

                    // STATUS responses
                    if (line.StartsWith("ArdSTATUS:FW:"))
                    {
                        string fw = line.Substring(13);
                        this.Invoke(new Action(() => {
                            statusFirmware.Text = "App v" + BUILD_VERSION + "  |  FW v" + fw;
                            if (fw != EXPECTED_FW_VERSION)
                                MessageBox.Show("Firmware mismatch.\nExpected: " + EXPECTED_FW_VERSION + "\nFound: " + fw,
                                    "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        }));
                        LogDebug("FW: " + fw);
                    }
                    else if (line.StartsWith("ArdSTATUS:MAC:"))
                    {
                        string mac = line.Substring(14);
                        this.Invoke(new Action(() => { lblMACValue.Text = mac; }));
                        LogDebug("MAC: " + mac);
                    }
                    else if (line.StartsWith("ArdSTATUS:IP:"))
                    {
                        string v = line.Substring(13);
                        this.Invoke(new Action(() => { txtIPAddress.Text = v; statusBoardIP.Text = "Board: " + v; }));
                    }
                    else if (line.StartsWith("ArdSTATUS:SUBNET:"))
                    {
                        this.Invoke(new Action(() => { txtSubnetMask.Text = line.Substring(17); }));
                    }
                    else if (line.StartsWith("ArdSTATUS:GATEWAY:"))
                    {
                        this.Invoke(new Action(() => { txtGateway.Text = line.Substring(18); }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R1TrigA:"))
                    {
                        string v = line.Substring(18);
                        this.Invoke(new Action(() => { txtTrig1A.Text = v; }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R1TrigB:"))
                    {
                        this.Invoke(new Action(() => { txtTrig1B.Text = line.Substring(18); }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R1TrigC:"))
                    {
                        this.Invoke(new Action(() => { txtTrig1C.Text = line.Substring(18); }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R2TrigA:"))
                    {
                        this.Invoke(new Action(() => { txtTrig2A.Text = line.Substring(18); }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R2TrigB:"))
                    {
                        this.Invoke(new Action(() => { txtTrig2B.Text = line.Substring(18); }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R2TrigC:"))
                    {
                        this.Invoke(new Action(() => { txtTrig2C.Text = line.Substring(18); }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R1TrigCount:") || line.StartsWith("ArdSTATUS:R2TrigCount:"))
                    {
                        // Informational — triggers are loaded individually
                    }
                    else if (line.StartsWith("ArdSTATUS:R1Mode:"))
                    {
                        bool latch = line.Contains("LATCH");
                        this.Invoke(new Action(() => { suppressEvents = true; cmbMode1.SelectedIndex = latch ? 1 : 0; suppressEvents = false; }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R2Mode:"))
                    {
                        bool latch = line.Contains("LATCH");
                        this.Invoke(new Action(() => { suppressEvents = true; cmbMode2.SelectedIndex = latch ? 1 : 0; suppressEvents = false; }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R1Duration:"))
                    {
                        int d; if (int.TryParse(line.Substring(21), out d))
                            this.Invoke(new Action(() => { suppressEvents = true; SelectDuration(cmbDuration1, d); suppressEvents = false; }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R2Duration:"))
                    {
                        int d; if (int.TryParse(line.Substring(21), out d))
                            this.Invoke(new Action(() => { suppressEvents = true; SelectDuration(cmbDuration2, d); suppressEvents = false; }));
                    }
                    else if (line.StartsWith("ArdSTATUS:R1State:"))
                    {
                        bool on = line.Contains("ON");
                        this.Invoke(new Action(() => SetRelayIndicator(1, on)));
                    }
                    else if (line.StartsWith("ArdSTATUS:R2State:"))
                    {
                        bool on = line.Contains("ON");
                        this.Invoke(new Action(() => SetRelayIndicator(2, on)));
                    }
                    else if (line.StartsWith("ArdSTATUS:VERBOSITY:"))
                    {
                        int v; if (int.TryParse(line.Substring(20), out v) && v >= 0 && v <= 2)
                            this.Invoke(new Action(() => { suppressEvents = true; cmbVerbosity.SelectedIndex = v; suppressEvents = false; }));
                    }
                    else if (line.StartsWith("ArdSTATUS:"))
                    {
                        LogDebug("Board: " + line.Substring(10));
                    }
                    // ACK responses
                    else if (line.StartsWith("ArdACK:RESET1:"))
                    {
                        this.Invoke(new Action(() => SetRelayIndicator(1, false)));
                        LogDebug("Relay 1 reset.");
                    }
                    else if (line.StartsWith("ArdACK:RESET2:"))
                    {
                        this.Invoke(new Action(() => SetRelayIndicator(2, false)));
                        LogDebug("Relay 2 reset.");
                    }
                    else if (line.StartsWith("ArdACK:TTEST"))
                    {
                        int r = line[12] - '0';
                        this.Invoke(new Action(() => SetRelayIndicator(r, true)));
                        LogDebug("Relay " + r + " timed test started.");
                    }
                    else if (line.StartsWith("ArdACK:TEST"))
                    {
                        int r = line[11] - '0';
                        this.Invoke(new Action(() => SetRelayIndicator(r, true)));
                        LogDebug("Relay " + r + " quick test started.");
                    }
                    else if (line.StartsWith("ArdACK:"))
                    {
                        LogDebug("Confirmed: " + line.Substring(7));
                    }
                    else if (line.StartsWith("ArdERR:"))
                    {
                        string err = line.Substring(7);
                        LogDebug("ERROR: " + err);
                        this.Invoke(new Action(() => MessageBox.Show("Board error:\n" + err, "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning)));
                    }
                    // Relay events
                    else if (line.Contains("test complete"))
                    {
                        if (line.Contains("Relay 1"))
                            this.Invoke(new Action(() => { SetRelayIndicator(1, false); btnTestRelay1.Enabled = true; btnTimedTestRelay1.Enabled = true; }));
                        else if (line.Contains("Relay 2"))
                            this.Invoke(new Action(() => { SetRelayIndicator(2, false); btnTestRelay2.Enabled = true; btnTimedTestRelay2.Enabled = true; }));
                        LogDebug(line);
                    }
                    else if (line.Contains("ENERGIZED"))
                    {
                        int r = line.Contains("Relay 1") ? 1 : 2;
                        this.Invoke(new Action(() => SetRelayIndicator(r, true)));
                        LogDebug("ALERT: " + line);
                    }
                    else if (line.Contains("de-energized"))
                    {
                        int r = line.Contains("Relay 1") ? 1 : 2;
                        this.Invoke(new Action(() => SetRelayIndicator(r, false)));
                        LogDebug(line);
                    }
                    else
                    {
                        LogDebug("Board: " + line);
                    }
                }
            }
            catch (TimeoutException) { }
            catch (Exception ex) { LogDebug("Serial error: " + ex.Message); }
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            try { if (serialPort?.IsOpen == true) serialPort.Close(); } catch { }
            try { logWriter?.WriteLine("=== Session ended " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + " ==="); logWriter?.Close(); } catch { }
            base.OnFormClosing(e);
        }
    }
}
