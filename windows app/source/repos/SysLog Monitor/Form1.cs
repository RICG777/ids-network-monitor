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

namespace SysLog_Monitor
{
    public partial class MainForm : Form
    {
        SerialPort serialPort = new SerialPort();

        public MainForm()
        {
            InitializeComponent();
        }

        public object textBoxTrigger { get; private set; }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (!serialPort.IsOpen)
            {
                try
                {
                    serialPort.PortName = "COMx"; // Replace 'x' with your board's COM port number
                    serialPort.BaudRate = 9600; // Baud rate should match the board's setting
                    serialPort.Open();
                    MessageBox.Show("Connected to board!");
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error connecting: " + ex.Message);
                }
            }
            else
            {
                MessageBox.Show("Already connected!");
            }
        }

        private void btnSetTrigger_Click(object sender, EventArgs e)
        {
            if (serialPort.IsOpen)
            {
                string message = textBoxTrigger.Text;
                serialPort.WriteLine(message);
                MessageBox.Show("Trigger message set!");
            }
            else
            {
                MessageBox.Show("Not connected to board!");
            }
        }
    }
}
