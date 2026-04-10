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
            this.grpBoardInfo = new System.Windows.Forms.GroupBox();
            this.lblMACLabel = new System.Windows.Forms.Label();
            this.lblMACValue = new System.Windows.Forms.Label();
            this.lblVerbLabel = new System.Windows.Forms.Label();
            this.cmbVerbosity = new System.Windows.Forms.ComboBox();
            this.grpNetwork = new System.Windows.Forms.GroupBox();
            this.lblIPAddress = new System.Windows.Forms.Label();
            this.txtIPAddress = new System.Windows.Forms.TextBox();
            this.lblIPError = new System.Windows.Forms.Label();
            this.lblSubnetMask = new System.Windows.Forms.Label();
            this.txtSubnetMask = new System.Windows.Forms.TextBox();
            this.lblGateway = new System.Windows.Forms.Label();
            this.txtGateway = new System.Windows.Forms.TextBox();
            this.btnSetNetwork = new System.Windows.Forms.Button();

            // Relay 1
            this.grpRelay1 = new System.Windows.Forms.GroupBox();
            this.pnlRelay1Indicator = new System.Windows.Forms.Panel();
            this.lblRelay1Status = new System.Windows.Forms.Label();
            this.btnResetRelay1 = new System.Windows.Forms.Button();
            this.btnTestRelay1 = new System.Windows.Forms.Button();
            this.btnTimedTestRelay1 = new System.Windows.Forms.Button();
            this.lblMode1 = new System.Windows.Forms.Label();
            this.cmbMode1 = new System.Windows.Forms.ComboBox();
            this.lblDuration1 = new System.Windows.Forms.Label();
            this.cmbDuration1 = new System.Windows.Forms.ComboBox();
            this.lblTrig1A = new System.Windows.Forms.Label();
            this.txtTrig1A = new System.Windows.Forms.TextBox();
            this.btnSetTrig1A = new System.Windows.Forms.Button();
            this.lblTrig1B = new System.Windows.Forms.Label();
            this.txtTrig1B = new System.Windows.Forms.TextBox();
            this.btnSetTrig1B = new System.Windows.Forms.Button();
            this.lblTrig1C = new System.Windows.Forms.Label();
            this.txtTrig1C = new System.Windows.Forms.TextBox();
            this.btnSetTrig1C = new System.Windows.Forms.Button();

            // Relay 2
            this.grpRelay2 = new System.Windows.Forms.GroupBox();
            this.pnlRelay2Indicator = new System.Windows.Forms.Panel();
            this.lblRelay2Status = new System.Windows.Forms.Label();
            this.btnResetRelay2 = new System.Windows.Forms.Button();
            this.btnTestRelay2 = new System.Windows.Forms.Button();
            this.btnTimedTestRelay2 = new System.Windows.Forms.Button();
            this.lblMode2 = new System.Windows.Forms.Label();
            this.cmbMode2 = new System.Windows.Forms.ComboBox();
            this.lblDuration2 = new System.Windows.Forms.Label();
            this.cmbDuration2 = new System.Windows.Forms.ComboBox();
            this.lblTrig2A = new System.Windows.Forms.Label();
            this.txtTrig2A = new System.Windows.Forms.TextBox();
            this.btnSetTrig2A = new System.Windows.Forms.Button();
            this.lblTrig2B = new System.Windows.Forms.Label();
            this.txtTrig2B = new System.Windows.Forms.TextBox();
            this.btnSetTrig2B = new System.Windows.Forms.Button();
            this.lblTrig2C = new System.Windows.Forms.Label();
            this.txtTrig2C = new System.Windows.Forms.TextBox();
            this.btnSetTrig2C = new System.Windows.Forms.Button();

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

            // ===================== CONNECTION TAB =====================
            this.tabConnection.Controls.Add(this.grpSerialPort);
            this.tabConnection.Controls.Add(this.grpBoardInfo);
            this.tabConnection.Controls.Add(this.grpNetwork);
            this.tabConnection.Padding = new System.Windows.Forms.Padding(12);
            this.tabConnection.Text = "Connection";
            this.tabConnection.UseVisualStyleBackColor = true;

            // Serial Port
            var topAnchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            this.grpSerialPort.Anchor = topAnchor;
            this.grpSerialPort.Controls.Add(this.lblSerialPort);
            this.grpSerialPort.Controls.Add(this.cmbCOMPorts);
            this.grpSerialPort.Controls.Add(this.btnRefreshPorts);
            this.grpSerialPort.Controls.Add(this.btnConnect);
            this.grpSerialPort.Controls.Add(this.btnAbout);
            this.grpSerialPort.Location = new System.Drawing.Point(15, 15);
            this.grpSerialPort.Size = new System.Drawing.Size(446, 60);
            this.grpSerialPort.Text = "Serial Port";

            this.lblSerialPort.AutoSize = true;
            this.lblSerialPort.Location = new System.Drawing.Point(15, 25);
            this.lblSerialPort.Text = "Port:";
            this.cmbCOMPorts.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbCOMPorts.Location = new System.Drawing.Point(50, 21);
            this.cmbCOMPorts.Size = new System.Drawing.Size(90, 21);
            this.btnRefreshPorts.Location = new System.Drawing.Point(148, 19);
            this.btnRefreshPorts.Size = new System.Drawing.Size(26, 25);
            this.btnRefreshPorts.Text = "\u21BB";
            this.btnRefreshPorts.Click += new System.EventHandler(this.btnRefreshPorts_Click);
            this.btnConnect.Location = new System.Drawing.Point(185, 19);
            this.btnConnect.Size = new System.Drawing.Size(100, 25);
            this.btnConnect.Text = "Connect";
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            this.btnAbout.Location = new System.Drawing.Point(350, 19);
            this.btnAbout.Size = new System.Drawing.Size(80, 25);
            this.btnAbout.Text = "About";
            this.btnAbout.Click += new System.EventHandler(this.btnAbout_Click);

            // Board Info
            this.grpBoardInfo.Anchor = topAnchor;
            this.grpBoardInfo.Controls.Add(this.lblMACLabel);
            this.grpBoardInfo.Controls.Add(this.lblMACValue);
            this.grpBoardInfo.Controls.Add(this.lblVerbLabel);
            this.grpBoardInfo.Controls.Add(this.cmbVerbosity);
            this.grpBoardInfo.Enabled = false;
            this.grpBoardInfo.Location = new System.Drawing.Point(15, 83);
            this.grpBoardInfo.Size = new System.Drawing.Size(446, 50);
            this.grpBoardInfo.Text = "Board Information";

            this.lblMACLabel.AutoSize = true;
            this.lblMACLabel.Location = new System.Drawing.Point(15, 22);
            this.lblMACLabel.Text = "MAC:";
            this.lblMACValue.AutoSize = true;
            this.lblMACValue.Font = new System.Drawing.Font("Consolas", 9F);
            this.lblMACValue.Location = new System.Drawing.Point(50, 22);
            this.lblMACValue.Text = "\u2014";
            this.lblVerbLabel.AutoSize = true;
            this.lblVerbLabel.Location = new System.Drawing.Point(230, 22);
            this.lblVerbLabel.Text = "Log level:";
            this.cmbVerbosity.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbVerbosity.Items.AddRange(new object[] { "Quiet", "Normal", "Debug" });
            this.cmbVerbosity.Location = new System.Drawing.Point(290, 18);
            this.cmbVerbosity.Size = new System.Drawing.Size(90, 21);
            this.cmbVerbosity.SelectedIndex = 1;
            this.cmbVerbosity.SelectedIndexChanged += new System.EventHandler(this.cmbVerbosity_Changed);

            // Network
            this.grpNetwork.Anchor = topAnchor;
            this.grpNetwork.Controls.Add(this.lblIPAddress);
            this.grpNetwork.Controls.Add(this.txtIPAddress);
            this.grpNetwork.Controls.Add(this.lblIPError);
            this.grpNetwork.Controls.Add(this.lblSubnetMask);
            this.grpNetwork.Controls.Add(this.txtSubnetMask);
            this.grpNetwork.Controls.Add(this.lblGateway);
            this.grpNetwork.Controls.Add(this.txtGateway);
            this.grpNetwork.Controls.Add(this.btnSetNetwork);
            this.grpNetwork.Enabled = false;
            this.grpNetwork.Location = new System.Drawing.Point(15, 141);
            this.grpNetwork.Size = new System.Drawing.Size(446, 160);
            this.grpNetwork.Text = "Network Configuration";

            this.lblIPAddress.AutoSize = true;
            this.lblIPAddress.Location = new System.Drawing.Point(15, 28);
            this.lblIPAddress.Text = "IP Address:";
            this.txtIPAddress.Location = new System.Drawing.Point(110, 25);
            this.txtIPAddress.Size = new System.Drawing.Size(140, 20);
            this.lblIPError.AutoSize = true;
            this.lblIPError.ForeColor = System.Drawing.Color.Red;
            this.lblIPError.Location = new System.Drawing.Point(260, 28);
            this.lblIPError.Text = "";
            this.lblSubnetMask.AutoSize = true;
            this.lblSubnetMask.Location = new System.Drawing.Point(15, 55);
            this.lblSubnetMask.Text = "Subnet Mask:";
            this.txtSubnetMask.Location = new System.Drawing.Point(110, 52);
            this.txtSubnetMask.Size = new System.Drawing.Size(140, 20);
            this.txtSubnetMask.Text = "255.255.255.0";
            this.lblGateway.AutoSize = true;
            this.lblGateway.Location = new System.Drawing.Point(15, 82);
            this.lblGateway.Text = "Gateway:";
            this.txtGateway.Location = new System.Drawing.Point(110, 79);
            this.txtGateway.Size = new System.Drawing.Size(140, 20);
            this.btnSetNetwork.Location = new System.Drawing.Point(110, 112);
            this.btnSetNetwork.Size = new System.Drawing.Size(140, 28);
            this.btnSetNetwork.Text = "Apply Network Settings";
            this.btnSetNetwork.Click += new System.EventHandler(this.btnSetNetwork_Click);

            // ===================== RELAYS TAB =====================
            this.tabRelays.Controls.Add(this.grpRelay1);
            this.tabRelays.Controls.Add(this.grpRelay2);
            this.tabRelays.Padding = new System.Windows.Forms.Padding(8);
            this.tabRelays.Text = "Relay Configuration";
            this.tabRelays.UseVisualStyleBackColor = true;
            this.tabRelays.AutoScroll = true;

            // Helper: create a relay group
            SetupRelayGroup(this.grpRelay1, "Relay 1", 10,
                this.pnlRelay1Indicator, this.lblRelay1Status,
                this.btnResetRelay1, this.btnTestRelay1, this.btnTimedTestRelay1,
                this.lblMode1, this.cmbMode1, this.lblDuration1, this.cmbDuration1,
                this.lblTrig1A, this.txtTrig1A, this.btnSetTrig1A,
                this.lblTrig1B, this.txtTrig1B, this.btnSetTrig1B,
                this.lblTrig1C, this.txtTrig1C, this.btnSetTrig1C,
                this.btnResetRelay1_Click, this.btnTestRelay1_Click, this.btnTimedTestRelay1_Click,
                this.cmbMode1_Changed, this.cmbDuration1_Changed,
                this.btnSetTrig1A_Click, this.btnSetTrig1B_Click, this.btnSetTrig1C_Click);

            SetupRelayGroup(this.grpRelay2, "Relay 2", 215,
                this.pnlRelay2Indicator, this.lblRelay2Status,
                this.btnResetRelay2, this.btnTestRelay2, this.btnTimedTestRelay2,
                this.lblMode2, this.cmbMode2, this.lblDuration2, this.cmbDuration2,
                this.lblTrig2A, this.txtTrig2A, this.btnSetTrig2A,
                this.lblTrig2B, this.txtTrig2B, this.btnSetTrig2B,
                this.lblTrig2C, this.txtTrig2C, this.btnSetTrig2C,
                this.btnResetRelay2_Click, this.btnTestRelay2_Click, this.btnTimedTestRelay2_Click,
                this.cmbMode2_Changed, this.cmbDuration2_Changed,
                this.btnSetTrig2A_Click, this.btnSetTrig2B_Click, this.btnSetTrig2C_Click);

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
            this.txtDebug.Size = new System.Drawing.Size(446, 350);
            this.txtDebug.WordWrap = false;
            this.btnClearLog.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
            this.btnClearLog.Location = new System.Drawing.Point(376, 373);
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
            this.statusFirmware.TextAlign = System.Drawing.ContentAlignment.MiddleRight;

            // ===================== FORM =====================
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 500);
            this.Controls.Add(this.tabControl);
            this.Controls.Add(this.statusStrip);
            this.MinimumSize = new System.Drawing.Size(500, 540);
            this.Text = "IDS Network Monitor";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        private void SetupRelayGroup(System.Windows.Forms.GroupBox grp, string title, int top,
            System.Windows.Forms.Panel indicator, System.Windows.Forms.Label statusLbl,
            System.Windows.Forms.Button btnReset, System.Windows.Forms.Button btnQuickTest, System.Windows.Forms.Button btnTimedTest,
            System.Windows.Forms.Label lblMode, System.Windows.Forms.ComboBox cmbMode,
            System.Windows.Forms.Label lblDur, System.Windows.Forms.ComboBox cmbDur,
            System.Windows.Forms.Label lblA, System.Windows.Forms.TextBox txtA, System.Windows.Forms.Button btnA,
            System.Windows.Forms.Label lblB, System.Windows.Forms.TextBox txtB, System.Windows.Forms.Button btnB,
            System.Windows.Forms.Label lblC, System.Windows.Forms.TextBox txtC, System.Windows.Forms.Button btnC,
            System.EventHandler resetClick, System.EventHandler quickClick, System.EventHandler timedClick,
            System.EventHandler modeChanged, System.EventHandler durChanged,
            System.EventHandler clickA, System.EventHandler clickB, System.EventHandler clickC)
        {
            grp.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
            grp.Controls.AddRange(new System.Windows.Forms.Control[] {
                indicator, statusLbl, btnReset, btnQuickTest, btnTimedTest,
                lblMode, cmbMode, lblDur, cmbDur,
                lblA, txtA, btnA, lblB, txtB, btnB, lblC, txtC, btnC });
            grp.Enabled = false;
            grp.Location = new System.Drawing.Point(12, top);
            grp.Size = new System.Drawing.Size(450, 195);
            grp.Text = title;

            // Row 1: status + buttons + mode
            indicator.BackColor = System.Drawing.Color.Gray;
            indicator.Location = new System.Drawing.Point(12, 20);
            indicator.Size = new System.Drawing.Size(16, 16);
            statusLbl.AutoSize = true;
            statusLbl.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold);
            statusLbl.Location = new System.Drawing.Point(34, 22);
            statusLbl.Text = "OFF";
            btnReset.Location = new System.Drawing.Point(90, 18);
            btnReset.Size = new System.Drawing.Size(55, 22);
            btnReset.Text = "Reset";
            btnReset.Click += resetClick;
            btnQuickTest.Location = new System.Drawing.Point(150, 18);
            btnQuickTest.Size = new System.Drawing.Size(70, 22);
            btnQuickTest.Text = "Quick Test";
            btnQuickTest.Click += quickClick;
            btnTimedTest.Location = new System.Drawing.Point(225, 18);
            btnTimedTest.Size = new System.Drawing.Size(80, 22);
            btnTimedTest.Text = "Timed Test";
            btnTimedTest.Click += timedClick;
            lblMode.AutoSize = true;
            lblMode.Location = new System.Drawing.Point(320, 22);
            lblMode.Text = "Mode:";
            cmbMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            cmbMode.Items.AddRange(new object[] { "Pulse", "Latch" });
            cmbMode.Location = new System.Drawing.Point(360, 18);
            cmbMode.Size = new System.Drawing.Size(75, 21);
            cmbMode.SelectedIndex = 0;
            cmbMode.SelectedIndexChanged += modeChanged;

            // Row 2: trigger A
            int trigY = 48;
            lblA.AutoSize = true;
            lblA.Location = new System.Drawing.Point(12, trigY + 3);
            lblA.Text = "Trigger A:";
            txtA.Location = new System.Drawing.Point(80, trigY);
            txtA.Size = new System.Drawing.Size(270, 20);
            txtA.MaxLength = 39;
            btnA.Location = new System.Drawing.Point(358, trigY - 2);
            btnA.Size = new System.Drawing.Size(78, 23);
            btnA.Text = "Apply";
            btnA.Click += clickA;

            // Row 3: trigger B
            trigY = 75;
            lblB.AutoSize = true;
            lblB.Location = new System.Drawing.Point(12, trigY + 3);
            lblB.Text = "Trigger B:";
            txtB.Location = new System.Drawing.Point(80, trigY);
            txtB.Size = new System.Drawing.Size(270, 20);
            txtB.MaxLength = 39;
            btnB.Location = new System.Drawing.Point(358, trigY - 2);
            btnB.Size = new System.Drawing.Size(78, 23);
            btnB.Text = "Apply";
            btnB.Click += clickB;

            // Row 4: trigger C
            trigY = 102;
            lblC.AutoSize = true;
            lblC.Location = new System.Drawing.Point(12, trigY + 3);
            lblC.Text = "Trigger C:";
            txtC.Location = new System.Drawing.Point(80, trigY);
            txtC.Size = new System.Drawing.Size(270, 20);
            txtC.MaxLength = 39;
            btnC.Location = new System.Drawing.Point(358, trigY - 2);
            btnC.Size = new System.Drawing.Size(78, 23);
            btnC.Text = "Apply";
            btnC.Click += clickC;

            // Row 5: duration
            lblDur.AutoSize = true;
            lblDur.Location = new System.Drawing.Point(12, 135);
            lblDur.Text = "Pulse duration:";
            cmbDur.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            cmbDur.Items.AddRange(new object[] { "5 seconds", "10 seconds", "15 seconds", "30 seconds", "60 seconds", "120 seconds" });
            cmbDur.Location = new System.Drawing.Point(110, 132);
            cmbDur.Size = new System.Drawing.Size(110, 21);
            cmbDur.SelectedIndex = 1;
            cmbDur.SelectedIndexChanged += durChanged;
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
        private System.Windows.Forms.GroupBox grpSerialPort;
        private System.Windows.Forms.Label lblSerialPort;
        private System.Windows.Forms.ComboBox cmbCOMPorts;
        private System.Windows.Forms.Button btnRefreshPorts;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.Button btnAbout;
        private System.Windows.Forms.GroupBox grpBoardInfo;
        private System.Windows.Forms.Label lblMACLabel;
        private System.Windows.Forms.Label lblMACValue;
        private System.Windows.Forms.Label lblVerbLabel;
        private System.Windows.Forms.ComboBox cmbVerbosity;
        private System.Windows.Forms.GroupBox grpNetwork;
        private System.Windows.Forms.Label lblIPAddress;
        private System.Windows.Forms.TextBox txtIPAddress;
        private System.Windows.Forms.Label lblIPError;
        private System.Windows.Forms.Label lblSubnetMask;
        private System.Windows.Forms.TextBox txtSubnetMask;
        private System.Windows.Forms.Label lblGateway;
        private System.Windows.Forms.TextBox txtGateway;
        private System.Windows.Forms.Button btnSetNetwork;
        private System.Windows.Forms.GroupBox grpRelay1;
        private System.Windows.Forms.Panel pnlRelay1Indicator;
        private System.Windows.Forms.Label lblRelay1Status;
        private System.Windows.Forms.Button btnResetRelay1;
        private System.Windows.Forms.Button btnTestRelay1;
        private System.Windows.Forms.Button btnTimedTestRelay1;
        private System.Windows.Forms.Label lblMode1;
        private System.Windows.Forms.ComboBox cmbMode1;
        private System.Windows.Forms.Label lblDuration1;
        private System.Windows.Forms.ComboBox cmbDuration1;
        private System.Windows.Forms.Label lblTrig1A;
        private System.Windows.Forms.TextBox txtTrig1A;
        private System.Windows.Forms.Button btnSetTrig1A;
        private System.Windows.Forms.Label lblTrig1B;
        private System.Windows.Forms.TextBox txtTrig1B;
        private System.Windows.Forms.Button btnSetTrig1B;
        private System.Windows.Forms.Label lblTrig1C;
        private System.Windows.Forms.TextBox txtTrig1C;
        private System.Windows.Forms.Button btnSetTrig1C;
        private System.Windows.Forms.GroupBox grpRelay2;
        private System.Windows.Forms.Panel pnlRelay2Indicator;
        private System.Windows.Forms.Label lblRelay2Status;
        private System.Windows.Forms.Button btnResetRelay2;
        private System.Windows.Forms.Button btnTestRelay2;
        private System.Windows.Forms.Button btnTimedTestRelay2;
        private System.Windows.Forms.Label lblMode2;
        private System.Windows.Forms.ComboBox cmbMode2;
        private System.Windows.Forms.Label lblDuration2;
        private System.Windows.Forms.ComboBox cmbDuration2;
        private System.Windows.Forms.Label lblTrig2A;
        private System.Windows.Forms.TextBox txtTrig2A;
        private System.Windows.Forms.Button btnSetTrig2A;
        private System.Windows.Forms.Label lblTrig2B;
        private System.Windows.Forms.TextBox txtTrig2B;
        private System.Windows.Forms.Button btnSetTrig2B;
        private System.Windows.Forms.Label lblTrig2C;
        private System.Windows.Forms.TextBox txtTrig2C;
        private System.Windows.Forms.Button btnSetTrig2C;
        private System.Windows.Forms.TextBox txtDebug;
        private System.Windows.Forms.Button btnClearLog;
    }
}
