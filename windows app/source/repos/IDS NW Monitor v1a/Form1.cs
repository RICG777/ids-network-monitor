using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
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

        private const string BUILD_VERSION = "0.3.0";  // v0.3.0 — bug fixes, ACK protocol, settings persistence

        public Form1()
        {
            InitializeComponent();

            // Populate COM ports
            cmbCOMPorts.Items.AddRange(SerialPort.GetPortNames());

            // Restore saved settings
            var settings = Properties.Settings.Default;

            // Select saved COM port if available, otherwise first port
            if (!string.IsNullOrEmpty(settings.COMPort) && cmbCOMPorts.Items.Contains(settings.COMPort))
            {
                cmbCOMPorts.SelectedItem = settings.COMPort;
            }
            else if (cmbCOMPorts.Items.Count > 0)
            {
                cmbCOMPorts.SelectedIndex = 0;
            }

            // Restore saved field values
            if (!string.IsNullOrEmpty(settings.IPAddress))
                txtIPAddress.Text = settings.IPAddress;
            if (!string.IsNullOrEmpty(settings.Trigger1))
                textBoxTrigger.Text = settings.Trigger1;
            if (!string.IsNullOrEmpty(settings.Trigger2))
                textBoxTrigger2.Text = settings.Trigger2;

            // Initialize serial port (only if a port is available)
            if (cmbCOMPorts.SelectedItem != null)
            {
                serialPort = new SerialPort(cmbCOMPorts.SelectedItem.ToString(), 9600);
            }
            else
            {
                serialPort = new SerialPort();
            }
            serialPort.DataReceived += serialPort_DataReceived;

            buildVersion.Text = "Build Version: " + BUILD_VERSION;
        }

        private void LogDebug(string message)
        {
            string timestamped = "[" + DateTime.Now.ToString("HH:mm:ss") + "] " + message + "\r\n";
            if (txtDebug.InvokeRequired)
            {
                txtDebug.Invoke(new Action(() => {
                    txtDebug.AppendText(timestamped);
                }));
            }
            else
            {
                txtDebug.AppendText(timestamped);
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (!serialPort.IsOpen)
            {
                if (cmbCOMPorts.SelectedItem == null)
                {
                    MessageBox.Show("Please select a COM port.");
                    return;
                }

                serialPort.PortName = cmbCOMPorts.SelectedItem.ToString();

                try
                {
                    LogDebug("Opening " + serialPort.PortName + "...");
                    serialPort.Open();
                    serialPort.DtrEnable = true;
                    serialPort.RtsEnable = true;
                    btnConnect.Text = "Disconnect";
                    lblConnectionStatus.Text = "Status: Connected";
                    lblConnectionStatus.ForeColor = Color.Green;

                    // Save COM port choice
                    Properties.Settings.Default.COMPort = serialPort.PortName;
                    Properties.Settings.Default.Save();

                    // Request current config from board
                    LogDebug("Requesting board status...");
                    serialPort.Write("STATUS\n");
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error connecting: " + ex.Message);
                    lblConnectionStatus.Text = "Status: Error";
                    lblConnectionStatus.ForeColor = Color.Red;
                }
            }
            else
            {
                LogDebug("Closing " + serialPort.PortName + "...");
                serialPort.Close();
                btnConnect.Text = "Connect to Board";
                lblConnectionStatus.Text = "Status: Disconnected";
                lblConnectionStatus.ForeColor = SystemColors.ControlText;
            }
        }

        private void btnSetTrigger_Click(object sender, EventArgs e)
        {
            if (serialPort.IsOpen)
            {
                string trigger = textBoxTrigger.Text.Trim();
                if (string.IsNullOrEmpty(trigger))
                {
                    MessageBox.Show("Please enter a trigger message.");
                    return;
                }
                if (trigger.Length >= 80)
                {
                    MessageBox.Show("Trigger message must be less than 80 characters.");
                    return;
                }

                string message = "Relay1:" + trigger + "\n";
                serialPort.Write(message);
                LogDebug("Sent: " + message.Trim());

                // Save setting
                Properties.Settings.Default.Trigger1 = trigger;
                Properties.Settings.Default.Save();
            }
            else
            {
                MessageBox.Show("Please connect to the board first.");
            }
        }

        private void btnSetTrigger2_Click(object sender, EventArgs e)
        {
            if (serialPort.IsOpen)
            {
                string trigger = textBoxTrigger2.Text.Trim();
                if (string.IsNullOrEmpty(trigger))
                {
                    MessageBox.Show("Please enter a trigger message.");
                    return;
                }
                if (trigger.Length >= 80)
                {
                    MessageBox.Show("Trigger message must be less than 80 characters.");
                    return;
                }

                string message = "Relay2:" + trigger + "\n";
                serialPort.Write(message);
                LogDebug("Sent: " + message.Trim());

                // Save setting
                Properties.Settings.Default.Trigger2 = trigger;
                Properties.Settings.Default.Save();
            }
            else
            {
                MessageBox.Show("Please connect to the board first.");
            }
        }

        private void serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                // Read all available data line by line
                while (serialPort.BytesToRead > 0)
                {
                    string line = serialPort.ReadLine().Trim();
                    if (string.IsNullOrEmpty(line)) continue;

                    // Handle STATUS responses — populate fields from board
                    if (line.StartsWith("ArdSTATUS:IP:"))
                    {
                        string val = line.Substring("ArdSTATUS:IP:".Length);
                        this.Invoke(new Action(() => { txtIPAddress.Text = val; }));
                        LogDebug("Board IP: " + val);
                    }
                    else if (line.StartsWith("ArdSTATUS:Relay1:"))
                    {
                        string val = line.Substring("ArdSTATUS:Relay1:".Length);
                        this.Invoke(new Action(() => { textBoxTrigger.Text = val; }));
                        LogDebug("Board Relay1 trigger: " + val);
                    }
                    else if (line.StartsWith("ArdSTATUS:Relay2:"))
                    {
                        string val = line.Substring("ArdSTATUS:Relay2:".Length);
                        this.Invoke(new Action(() => { textBoxTrigger2.Text = val; }));
                        LogDebug("Board Relay2 trigger: " + val);
                    }
                    else if (line.StartsWith("ArdSTATUS:"))
                    {
                        LogDebug("Board: " + line.Substring("ArdSTATUS:".Length));
                    }
                    else if (line.StartsWith("ArdACK:"))
                    {
                        LogDebug("Confirmed: " + line.Substring("ArdACK:".Length));
                    }
                    else if (line.StartsWith("ArdERR:"))
                    {
                        string err = line.Substring("ArdERR:".Length);
                        LogDebug("ERROR from board: " + err);
                        this.Invoke(new Action(() => {
                            MessageBox.Show("Board error: " + err, "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        }));
                    }
                    else
                    {
                        LogDebug("Board: " + line);
                    }
                }
            }
            catch (TimeoutException)
            {
                // ReadLine timed out — no complete line available yet, will retry on next event
            }
            catch (Exception ex)
            {
                LogDebug("Serial error: " + ex.Message);
            }
        }

        private bool IsValidIPAddress(string ip)
        {
            string pattern = @"^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
            return Regex.IsMatch(ip, pattern);
        }

        private void btnSetIP_Click(object sender, EventArgs e)
        {
            if (IsValidIPAddress(txtIPAddress.Text))
            {
                if (serialPort.IsOpen)
                {
                    string message = "IP:" + txtIPAddress.Text + "\n";
                    serialPort.Write(message);
                    LogDebug("Sent: " + message.Trim());

                    // Save setting
                    Properties.Settings.Default.IPAddress = txtIPAddress.Text;
                    Properties.Settings.Default.Save();
                }
                else
                {
                    MessageBox.Show("Please connect to the board first.");
                }
            }
            else
            {
                MessageBox.Show("Please enter a valid IP address.");
            }
        }

        private void label2_Click(object sender, EventArgs e)
        {
        }

        private void buildVersion_Click(object sender, EventArgs e)
        {
        }
    }
}
