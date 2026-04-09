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
        // Declare the SerialPort object at the class level
        private SerialPort serialPort;

        private const string BUILD_VERSION = "0.2.1";  // Update this value for each new build



        public Form1()
        {
            InitializeComponent();
            // Initialize the SerialPort object
            cmbCOMPorts.Items.AddRange(SerialPort.GetPortNames());
            if (cmbCOMPorts.Items.Count > 0)
            {
                cmbCOMPorts.SelectedIndex = 0; // Select the first COM port by default
            }

            serialPort = new SerialPort(cmbCOMPorts.SelectedItem.ToString(), 9600);
            serialPort.DataReceived += serialPort_DataReceived; // Attach the event handler
            buildVersion.Text = "Build Version: " + BUILD_VERSION;
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (!serialPort.IsOpen)
            {

                // Check if a COM port is selected before trying to connect
                if (cmbCOMPorts.SelectedItem == null)
                {
                    MessageBox.Show("Please select a COM port.");
                    return; // Exit the method early
                }

                // Set the port name to the selected COM port
                serialPort.PortName = cmbCOMPorts.SelectedItem.ToString();

                try
                {
                    txtDebug.AppendText("Trying to open the port..."); // Debugging message

                    serialPort.Open();
                    serialPort.DtrEnable = true;
                    serialPort.RtsEnable = true;
                    btnConnect.Text = "Disconnect";
                    lblConnectionStatus.Text = "Status: Connected";
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error connecting to the board: " + ex.Message + "\n\n" + ex.StackTrace);
                    lblConnectionStatus.Text = "Status: Error";
                }
            }
            else
            {
                txtDebug.AppendText("Trying to close the port..."); // Debugging message

                serialPort.Close();
                btnConnect.Text = "Connect to Board";
                lblConnectionStatus.Text = "Status: Disconnected";
            }
        }


        private void btnSetTrigger_Click(object sender, EventArgs e)
        {
            txtDebug.AppendText("Set Trigger for Relay 1 button clicked.\n");

            if (serialPort.IsOpen)
            {
                // Clear the buffer
                while (serialPort.BytesToRead > 0)
                {
                    serialPort.ReadByte();
                }

                string message = "Relay1:" + textBoxTrigger.Text + "\n";
                serialPort.Write(message);
                txtDebug.AppendText("Message sent to board for Relay1: " + message + "\n");
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
                // Clear the buffer
                while (serialPort.BytesToRead > 0)
                {
                    serialPort.ReadByte();
                }

                string receivedData = serialPort.ReadLine();
                this.Invoke(new Action(() => {
                    txtDebug.AppendText("Received from board: " + receivedData + "\n");
                }));
            }
            catch (Exception ex)
            {
                this.Invoke(new Action(() => {
                    txtDebug.AppendText("Error reading from board: " + ex.Message + "\n");
                }));
            }
        }

        private bool IsValidIPAddress(string ip)
        {
            // Regular expression to validate IP address format
            string pattern = @"^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
            return Regex.IsMatch(ip, pattern);
        }

        private void btnSetIP_Click(object sender, EventArgs e)
        {
            if (IsValidIPAddress(txtIPAddress.Text))
            {
                if (serialPort.IsOpen)
                {
                    string newIPAddress = "IP:" + txtIPAddress.Text + "\n"; // Prefix the IP address with "IP:"
                    serialPort.Write(newIPAddress);
                    txtDebug.AppendText("New IP address sent to board: " + newIPAddress + "\n");
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

        private void btnSetTrigger2_Click(object sender, EventArgs e)
        {
            txtDebug.AppendText("Set Trigger for Relay 2 button clicked.\n");

            if (serialPort.IsOpen)
            {
                // Clear the buffer
                while (serialPort.BytesToRead > 0)
                {
                    serialPort.ReadByte();
                }

                string message = "Relay2:" + textBoxTrigger2.Text + "\n"; // Prefix with "Relay2:"
                serialPort.Write(message);
                txtDebug.AppendText("Message sent to board for Relay 2: " + message + "\n");
            }
            else
            {
                MessageBox.Show("Please connect to the board first.");
            }
        }

        private void buildVersion_Click(object sender, EventArgs e)
        {
            
        }
    }
}

