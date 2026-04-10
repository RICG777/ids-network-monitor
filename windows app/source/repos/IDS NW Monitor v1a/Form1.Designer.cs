namespace IDS_NW_Monitor_v1a
{
    partial class Form1
    {
        private System.ComponentModel.IContainer components = null;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
                components.Dispose();
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        private void InitializeComponent()
        {
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabConnection = new System.Windows.Forms.TabPage();
            this.tabRelays = new System.Windows.Forms.TabPage();
            this.tabLog = new System.Windows.Forms.TabPage();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.statusConnection = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusCOMPort = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusBoardIP = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusFirmware = new System.Windows.Forms.ToolStripStatusLabel();

            // Connection tab
            this.grpSerialPort = new System.Windows.Forms.GroupBox();
            this.lblSerialPort = new System.Windows.Forms.Label();
            this.cmbCOMPorts = new System.Windows.Forms.ComboBox();
            this.btnRefreshPorts = new System.Windows.Forms.Button();
            this.btnConnect = new System.Windows.Forms.Button();
            this.btnAbout = new System.Windows.Forms.Button();
            this.grpNetwork = new System.Windows.Forms.GroupBox();
            this.lblIPAddress = new System.Windows.Forms.Label();
            this.txtIPAddress = new System.Windows.Forms.TextBox();
            this.lblIPError = new System.Windows.Forms.Label();
            this.lblSubnetMask = new System.Windows.Forms.Label();
            this.txtSubnetMask = new System.Windows.Forms.TextBox();
            this.lblGateway = new System.Windows.Forms.Label();
            this.txtGateway = new System.Windows.Forms.TextBox();
            this.btnSetNetwork = new System.Windows.Forms.Button();
            this.grpBoardInfo = new System.Windows.Forms.GroupBox();
            this.lblMACLabel = new System.Windows.Forms.Label();
            this.lblMACValue = new System.Windows.Forms.Label();

            // Relay tab
            this.grpRelay1 = new System.Windows.Forms.GroupBox();
            this.pnlRelay1Indicator = new System.Windows.Forms.Panel();
            this.lblRelay1Status = new System.Windows.Forms.Label();
            this.lblTrigger1 = new System.Windows.Forms.Label();
            this.textBoxTrigger = new System.Windows.Forms.TextBox();
            this.lblTrigger1Error = new System.Windows.Forms.Label();
            this.btnSetTrigger = new System.Windows.Forms.Button();
            this.btnTestRelay1 = new System.Windows.Forms.Button();
            this.btnResetRelay1 = new System.Windows.Forms.Button();
            this.lblMode1 = new System.Windows.Forms.Label();
            this.cmbMode1 = new System.Windows.Forms.ComboBox();
            this.lblDuration1 = new System.Windows.Forms.Label();
            this.cmbDuration1 = new System.Windows.Forms.ComboBox();

            this.grpRelay2 = new System.Windows.Forms.GroupBox();
            this.pnlRelay2Indicator = new System.Windows.Forms.Panel();
            this.lblRelay2Status = new System.Windows.Forms.Label();
            this.lblTrigger2 = new System.Windows.Forms.Label();
            this.textBoxTrigger2 = new System.Windows.Forms.TextBox();
            this.lblTrigger2Error = new System.Windows.Forms.Label();
            this.btnSetTrigger2 = new System.Windows.Forms.Button();
            this.btnTestRelay2 = new System.Windows.Forms.Button();
            this.btnResetRelay2 = new System.Windows.Forms.Button();
            this.lblMode2 = new System.Windows.Forms.Label();
            this.cmbMode2 = new System.Windows.Forms.ComboBox();
            this.lblDuration2 = new System.Windows.Forms.Label();
            this.cmbDuration2 = new System.Windows.Forms.ComboBox();

            // Log tab
            this.txtDebug = new System.Windows.Forms.TextBox();
            this.btnClearLog = new System.Windows.Forms.Button();

            this.SuspendLayout();

            // ===================== TAB CONTROL =====================
            this.tabControl.Controls.Add(this.tabConnection);
            this.tabControl.Controls.Add(this.tabRelays);
            this.tabControl.Controls.Add(this.tabLog);
            this.tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl.Padding = new System.Drawing.Point(12, 6);
            this.tabControl.TabIndex = 0;

            // ===================== CONNECTION TAB =====================
            this.tabConnection.Controls.Add(this.grpSerialPort);
            this.tabConnection.Controls.Add(this.grpNetwork);
            this.tabConnection.Controls.Add(this.grpBoardInfo);
            this.tabConnection.Padding = new System.Windows.Forms.Padding(12);
            this.tabConnection.Text = "Connection";
            this.tabConnection.UseVisualStyleBackColor = true;

            // Serial Port group
            this.grpSerialPort.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            this.grpSerialPort.Controls.Add(this.lblSerialPort);
            this.grpSerialPort.Controls.Add(this.cmbCOMPorts);
            this.grpSerialPort.Controls.Add(this.btnRefreshPorts);
            this.grpSerialPort.Controls.Add(this.btnConnect);
            this.grpSerialPort.Controls.Add(this.btnAbout);
            this.grpSerialPort.Location = new System.Drawing.Point(15, 15);
            this.grpSerialPort.Size = new System.Drawing.Size(446, 65);
            this.grpSerialPort.Text = "Serial Port";

            this.lblSerialPort.AutoSize = true;
            this.lblSerialPort.Location = new System.Drawing.Point(15, 28);
            this.lblSerialPort.Text = "Port:";

            this.cmbCOMPorts.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbCOMPorts.Location = new System.Drawing.Point(50, 24);
            this.cmbCOMPorts.Size = new System.Drawing.Size(100, 21);

            this.btnRefreshPorts.Location = new System.Drawing.Point(158, 22);
            this.btnRefreshPorts.Size = new System.Drawing.Size(26, 25);
            this.btnRefreshPorts.Text = "\u21BB";
            this.btnRefreshPorts.Click += new System.EventHandler(this.btnRefreshPorts_Click);

            this.btnConnect.Location = new System.Drawing.Point(200, 22);
            this.btnConnect.Size = new System.Drawing.Size(110, 25);
            this.btnConnect.Text = "Connect";
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);

            this.btnAbout.Location = new System.Drawing.Point(340, 22);
            this.btnAbout.Size = new System.Drawing.Size(90, 25);
            this.btnAbout.Text = "About";
            this.btnAbout.Click += new System.EventHandler(this.btnAbout_Click);

            // Board Info group
            this.grpBoardInfo.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            this.grpBoardInfo.Controls.Add(this.lblMACLabel);
            this.grpBoardInfo.Controls.Add(this.lblMACValue);
            this.grpBoardInfo.Enabled = false;
            this.grpBoardInfo.Location = new System.Drawing.Point(15, 90);
            this.grpBoardInfo.Size = new System.Drawing.Size(446, 50);
            this.grpBoardInfo.Text = "Board Information";

            this.lblMACLabel.AutoSize = true;
            this.lblMACLabel.Location = new System.Drawing.Point(15, 22);
            this.lblMACLabel.Text = "MAC Address:";

            this.lblMACValue.AutoSize = true;
            this.lblMACValue.Font = new System.Drawing.Font("Consolas", 9.5F);
            this.lblMACValue.Location = new System.Drawing.Point(120, 22);
            this.lblMACValue.Text = "—";

            // Network group
            this.grpNetwork.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            this.grpNetwork.Controls.Add(this.lblIPAddress);
            this.grpNetwork.Controls.Add(this.txtIPAddress);
            this.grpNetwork.Controls.Add(this.lblIPError);
            this.grpNetwork.Controls.Add(this.lblSubnetMask);
            this.grpNetwork.Controls.Add(this.txtSubnetMask);
            this.grpNetwork.Controls.Add(this.lblGateway);
            this.grpNetwork.Controls.Add(this.txtGateway);
            this.grpNetwork.Controls.Add(this.btnSetNetwork);
            this.grpNetwork.Enabled = false;
            this.grpNetwork.Location = new System.Drawing.Point(15, 150);
            this.grpNetwork.Size = new System.Drawing.Size(446, 160);
            this.grpNetwork.Text = "Network Configuration";

            this.lblIPAddress.AutoSize = true;
            this.lblIPAddress.Location = new System.Drawing.Point(15, 30);
            this.lblIPAddress.Text = "IP Address:";

            this.txtIPAddress.Location = new System.Drawing.Point(120, 27);
            this.txtIPAddress.Size = new System.Drawing.Size(150, 20);

            this.lblIPError.AutoSize = true;
            this.lblIPError.ForeColor = System.Drawing.Color.Red;
            this.lblIPError.Location = new System.Drawing.Point(276, 30);
            this.lblIPError.Text = "";

            this.lblSubnetMask.AutoSize = true;
            this.lblSubnetMask.Location = new System.Drawing.Point(15, 60);
            this.lblSubnetMask.Text = "Subnet Mask:";

            this.txtSubnetMask.Location = new System.Drawing.Point(120, 57);
            this.txtSubnetMask.Size = new System.Drawing.Size(150, 20);
            this.txtSubnetMask.Text = "255.255.255.0";

            this.lblGateway.AutoSize = true;
            this.lblGateway.Location = new System.Drawing.Point(15, 90);
            this.lblGateway.Text = "Gateway:";

            this.txtGateway.Location = new System.Drawing.Point(120, 87);
            this.txtGateway.Size = new System.Drawing.Size(150, 20);

            this.btnSetNetwork.Location = new System.Drawing.Point(120, 120);
            this.btnSetNetwork.Size = new System.Drawing.Size(150, 28);
            this.btnSetNetwork.Text = "Apply Network Settings";
            this.btnSetNetwork.Click += new System.EventHandler(this.btnSetIP_Click);

            // ===================== RELAYS TAB =====================
            this.tabRelays.Controls.Add(this.grpRelay1);
            this.tabRelays.Controls.Add(this.grpRelay2);
            this.tabRelays.Padding = new System.Windows.Forms.Padding(12);
            this.tabRelays.Text = "Relay Configuration";
            this.tabRelays.UseVisualStyleBackColor = true;

            // --- Relay 1 ---
            this.grpRelay1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            this.grpRelay1.Controls.Add(this.pnlRelay1Indicator);
            this.grpRelay1.Controls.Add(this.lblRelay1Status);
            this.grpRelay1.Controls.Add(this.lblTrigger1);
            this.grpRelay1.Controls.Add(this.textBoxTrigger);
            this.grpRelay1.Controls.Add(this.lblTrigger1Error);
            this.grpRelay1.Controls.Add(this.btnSetTrigger);
            this.grpRelay1.Controls.Add(this.btnTestRelay1);
            this.grpRelay1.Controls.Add(this.btnResetRelay1);
            this.grpRelay1.Controls.Add(this.lblMode1);
            this.grpRelay1.Controls.Add(this.cmbMode1);
            this.grpRelay1.Controls.Add(this.lblDuration1);
            this.grpRelay1.Controls.Add(this.cmbDuration1);
            this.grpRelay1.Enabled = false;
            this.grpRelay1.Location = new System.Drawing.Point(15, 15);
            this.grpRelay1.Size = new System.Drawing.Size(446, 160);
            this.grpRelay1.Text = "Relay 1";

            this.pnlRelay1Indicator.BackColor = System.Drawing.Color.Gray;
            this.pnlRelay1Indicator.Location = new System.Drawing.Point(15, 22);
            this.pnlRelay1Indicator.Size = new System.Drawing.Size(16, 16);

            this.lblRelay1Status.AutoSize = true;
            this.lblRelay1Status.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold);
            this.lblRelay1Status.Location = new System.Drawing.Point(37, 24);
            this.lblRelay1Status.Text = "OFF";

            this.btnResetRelay1.Location = new System.Drawing.Point(120, 20);
            this.btnResetRelay1.Size = new System.Drawing.Size(70, 22);
            this.btnResetRelay1.Text = "Reset";
            this.btnResetRelay1.Click += new System.EventHandler(this.btnResetRelay1_Click);

            this.btnTestRelay1.Location = new System.Drawing.Point(200, 20);
            this.btnTestRelay1.Size = new System.Drawing.Size(80, 22);
            this.btnTestRelay1.Text = "Test Relay";
            this.btnTestRelay1.Click += new System.EventHandler(this.btnTestRelay1_Click);

            this.lblMode1.AutoSize = true;
            this.lblMode1.Location = new System.Drawing.Point(300, 24);
            this.lblMode1.Text = "Mode:";

            this.cmbMode1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbMode1.Items.AddRange(new object[] { "Pulse", "Latch" });
            this.cmbMode1.Location = new System.Drawing.Point(340, 20);
            this.cmbMode1.Size = new System.Drawing.Size(90, 21);
            this.cmbMode1.SelectedIndex = 0;
            this.cmbMode1.SelectedIndexChanged += new System.EventHandler(this.cmbMode1_Changed);

            this.lblTrigger1.AutoSize = true;
            this.lblTrigger1.Location = new System.Drawing.Point(15, 58);
            this.lblTrigger1.Text = "Trigger message:";

            this.textBoxTrigger.Location = new System.Drawing.Point(120, 55);
            this.textBoxTrigger.Size = new System.Drawing.Size(200, 20);
            this.textBoxTrigger.MaxLength = 79;

            this.btnSetTrigger.Location = new System.Drawing.Point(330, 53);
            this.btnSetTrigger.Size = new System.Drawing.Size(100, 24);
            this.btnSetTrigger.Text = "Apply Trigger";
            this.btnSetTrigger.Click += new System.EventHandler(this.btnSetTrigger_Click);

            this.lblTrigger1Error.AutoSize = true;
            this.lblTrigger1Error.ForeColor = System.Drawing.Color.Red;
            this.lblTrigger1Error.Location = new System.Drawing.Point(120, 78);
            this.lblTrigger1Error.Text = "";

            this.lblDuration1.AutoSize = true;
            this.lblDuration1.Location = new System.Drawing.Point(15, 100);
            this.lblDuration1.Text = "Pulse duration:";

            this.cmbDuration1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbDuration1.Items.AddRange(new object[] { "5 seconds", "10 seconds", "15 seconds", "30 seconds", "60 seconds", "120 seconds" });
            this.cmbDuration1.Location = new System.Drawing.Point(120, 97);
            this.cmbDuration1.Size = new System.Drawing.Size(120, 21);
            this.cmbDuration1.SelectedIndex = 1;
            this.cmbDuration1.SelectedIndexChanged += new System.EventHandler(this.cmbDuration1_Changed);

            // --- Relay 2 ---
            this.grpRelay2.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            this.grpRelay2.Controls.Add(this.pnlRelay2Indicator);
            this.grpRelay2.Controls.Add(this.lblRelay2Status);
            this.grpRelay2.Controls.Add(this.lblTrigger2);
            this.grpRelay2.Controls.Add(this.textBoxTrigger2);
            this.grpRelay2.Controls.Add(this.lblTrigger2Error);
            this.grpRelay2.Controls.Add(this.btnSetTrigger2);
            this.grpRelay2.Controls.Add(this.btnTestRelay2);
            this.grpRelay2.Controls.Add(this.btnResetRelay2);
            this.grpRelay2.Controls.Add(this.lblMode2);
            this.grpRelay2.Controls.Add(this.cmbMode2);
            this.grpRelay2.Controls.Add(this.lblDuration2);
            this.grpRelay2.Controls.Add(this.cmbDuration2);
            this.grpRelay2.Enabled = false;
            this.grpRelay2.Location = new System.Drawing.Point(15, 185);
            this.grpRelay2.Size = new System.Drawing.Size(446, 160);
            this.grpRelay2.Text = "Relay 2";

            this.pnlRelay2Indicator.BackColor = System.Drawing.Color.Gray;
            this.pnlRelay2Indicator.Location = new System.Drawing.Point(15, 22);
            this.pnlRelay2Indicator.Size = new System.Drawing.Size(16, 16);

            this.lblRelay2Status.AutoSize = true;
            this.lblRelay2Status.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold);
            this.lblRelay2Status.Location = new System.Drawing.Point(37, 24);
            this.lblRelay2Status.Text = "OFF";

            this.btnResetRelay2.Location = new System.Drawing.Point(120, 20);
            this.btnResetRelay2.Size = new System.Drawing.Size(70, 22);
            this.btnResetRelay2.Text = "Reset";
            this.btnResetRelay2.Click += new System.EventHandler(this.btnResetRelay2_Click);

            this.btnTestRelay2.Location = new System.Drawing.Point(200, 20);
            this.btnTestRelay2.Size = new System.Drawing.Size(80, 22);
            this.btnTestRelay2.Text = "Test Relay";
            this.btnTestRelay2.Click += new System.EventHandler(this.btnTestRelay2_Click);

            this.lblMode2.AutoSize = true;
            this.lblMode2.Location = new System.Drawing.Point(300, 24);
            this.lblMode2.Text = "Mode:";

            this.cmbMode2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbMode2.Items.AddRange(new object[] { "Pulse", "Latch" });
            this.cmbMode2.Location = new System.Drawing.Point(340, 20);
            this.cmbMode2.Size = new System.Drawing.Size(90, 21);
            this.cmbMode2.SelectedIndex = 0;
            this.cmbMode2.SelectedIndexChanged += new System.EventHandler(this.cmbMode2_Changed);

            this.lblTrigger2.AutoSize = true;
            this.lblTrigger2.Location = new System.Drawing.Point(15, 58);
            this.lblTrigger2.Text = "Trigger message:";

            this.textBoxTrigger2.Location = new System.Drawing.Point(120, 55);
            this.textBoxTrigger2.Size = new System.Drawing.Size(200, 20);
            this.textBoxTrigger2.MaxLength = 79;

            this.btnSetTrigger2.Location = new System.Drawing.Point(330, 53);
            this.btnSetTrigger2.Size = new System.Drawing.Size(100, 24);
            this.btnSetTrigger2.Text = "Apply Trigger";
            this.btnSetTrigger2.Click += new System.EventHandler(this.btnSetTrigger2_Click);

            this.lblTrigger2Error.AutoSize = true;
            this.lblTrigger2Error.ForeColor = System.Drawing.Color.Red;
            this.lblTrigger2Error.Location = new System.Drawing.Point(120, 78);
            this.lblTrigger2Error.Text = "";

            this.lblDuration2.AutoSize = true;
            this.lblDuration2.Location = new System.Drawing.Point(15, 100);
            this.lblDuration2.Text = "Pulse duration:";

            this.cmbDuration2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbDuration2.Items.AddRange(new object[] { "5 seconds", "10 seconds", "15 seconds", "30 seconds", "60 seconds", "120 seconds" });
            this.cmbDuration2.Location = new System.Drawing.Point(120, 97);
            this.cmbDuration2.Size = new System.Drawing.Size(120, 21);
            this.cmbDuration2.SelectedIndex = 1;
            this.cmbDuration2.SelectedIndexChanged += new System.EventHandler(this.cmbDuration2_Changed);

            // ===================== LOG TAB =====================
            this.tabLog.Controls.Add(this.txtDebug);
            this.tabLog.Controls.Add(this.btnClearLog);
            this.tabLog.Padding = new System.Windows.Forms.Padding(12);
            this.tabLog.Text = "Event Log";
            this.tabLog.UseVisualStyleBackColor = true;

            this.txtDebug.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            this.txtDebug.BackColor = System.Drawing.Color.FromArgb(30, 30, 30);
            this.txtDebug.Font = new System.Drawing.Font("Consolas", 9F);
            this.txtDebug.ForeColor = System.Drawing.Color.FromArgb(220, 220, 220);
            this.txtDebug.Location = new System.Drawing.Point(15, 15);
            this.txtDebug.Multiline = true;
            this.txtDebug.ReadOnly = true;
            this.txtDebug.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.txtDebug.Size = new System.Drawing.Size(446, 289);
            this.txtDebug.WordWrap = false;

            this.btnClearLog.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
            this.btnClearLog.Location = new System.Drawing.Point(376, 312);
            this.btnClearLog.Size = new System.Drawing.Size(85, 25);
            this.btnClearLog.Text = "Clear Log";
            this.btnClearLog.Click += new System.EventHandler(this.btnClearLog_Click);

            // ===================== STATUS STRIP =====================
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
                this.statusConnection, this.statusCOMPort, this.statusBoardIP, this.statusFirmware });

            this.statusConnection.AutoSize = false;
            this.statusConnection.BackColor = System.Drawing.Color.FromArgb(180, 0, 0);
            this.statusConnection.ForeColor = System.Drawing.Color.White;
            this.statusConnection.Size = new System.Drawing.Size(90, 17);
            this.statusConnection.Text = " Disconnected";
            this.statusConnection.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;

            this.statusCOMPort.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Left;
            this.statusCOMPort.Text = "";

            this.statusBoardIP.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Left;
            this.statusBoardIP.Text = "";

            this.statusFirmware.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Left;
            this.statusFirmware.Spring = true;
            this.statusFirmware.Text = "App v0.6.0";
            this.statusFirmware.TextAlign = System.Drawing.ContentAlignment.MiddleRight;

            // ===================== FORM =====================
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 440);
            this.Controls.Add(this.tabControl);
            this.Controls.Add(this.statusStrip);
            this.MinimumSize = new System.Drawing.Size(500, 480);
            this.Text = "IDS Network Monitor";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        #endregion

        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabConnection;
        private System.Windows.Forms.TabPage tabRelays;
        private System.Windows.Forms.TabPage tabLog;
        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripStatusLabel statusConnection;
        private System.Windows.Forms.ToolStripStatusLabel statusCOMPort;
        private System.Windows.Forms.ToolStripStatusLabel statusBoardIP;
        private System.Windows.Forms.ToolStripStatusLabel statusFirmware;

        // Connection tab
        private System.Windows.Forms.GroupBox grpSerialPort;
        private System.Windows.Forms.Label lblSerialPort;
        private System.Windows.Forms.ComboBox cmbCOMPorts;
        private System.Windows.Forms.Button btnRefreshPorts;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.Button btnAbout;
        private System.Windows.Forms.GroupBox grpBoardInfo;
        private System.Windows.Forms.Label lblMACLabel;
        private System.Windows.Forms.Label lblMACValue;
        private System.Windows.Forms.GroupBox grpNetwork;
        private System.Windows.Forms.Label lblIPAddress;
        private System.Windows.Forms.TextBox txtIPAddress;
        private System.Windows.Forms.Label lblIPError;
        private System.Windows.Forms.Label lblSubnetMask;
        private System.Windows.Forms.TextBox txtSubnetMask;
        private System.Windows.Forms.Label lblGateway;
        private System.Windows.Forms.TextBox txtGateway;
        private System.Windows.Forms.Button btnSetNetwork;

        // Relay 1
        private System.Windows.Forms.GroupBox grpRelay1;
        private System.Windows.Forms.Panel pnlRelay1Indicator;
        private System.Windows.Forms.Label lblRelay1Status;
        private System.Windows.Forms.Label lblTrigger1;
        private System.Windows.Forms.TextBox textBoxTrigger;
        private System.Windows.Forms.Label lblTrigger1Error;
        private System.Windows.Forms.Button btnSetTrigger;
        private System.Windows.Forms.Button btnTestRelay1;
        private System.Windows.Forms.Button btnResetRelay1;
        private System.Windows.Forms.Label lblMode1;
        private System.Windows.Forms.ComboBox cmbMode1;
        private System.Windows.Forms.Label lblDuration1;
        private System.Windows.Forms.ComboBox cmbDuration1;

        // Relay 2
        private System.Windows.Forms.GroupBox grpRelay2;
        private System.Windows.Forms.Panel pnlRelay2Indicator;
        private System.Windows.Forms.Label lblRelay2Status;
        private System.Windows.Forms.Label lblTrigger2;
        private System.Windows.Forms.TextBox textBoxTrigger2;
        private System.Windows.Forms.Label lblTrigger2Error;
        private System.Windows.Forms.Button btnSetTrigger2;
        private System.Windows.Forms.Button btnTestRelay2;
        private System.Windows.Forms.Button btnResetRelay2;
        private System.Windows.Forms.Label lblMode2;
        private System.Windows.Forms.ComboBox cmbMode2;
        private System.Windows.Forms.Label lblDuration2;
        private System.Windows.Forms.ComboBox cmbDuration2;

        // Log tab
        private System.Windows.Forms.TextBox txtDebug;
        private System.Windows.Forms.Button btnClearLog;
    }
}
