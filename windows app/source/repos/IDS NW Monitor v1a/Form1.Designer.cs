namespace IDS_NW_Monitor_v1a
{
    partial class Form1
    {
        private System.ComponentModel.IContainer components = null;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabConnection = new System.Windows.Forms.TabPage();
            this.tabRelays = new System.Windows.Forms.TabPage();
            this.tabLog = new System.Windows.Forms.TabPage();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.statusConnection = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusCOMPort = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusBoardIP = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusFirmware = new System.Windows.Forms.ToolStripStatusLabel();

            // Connection tab controls
            this.grpSerialPort = new System.Windows.Forms.GroupBox();
            this.lblSerialPort = new System.Windows.Forms.Label();
            this.cmbCOMPorts = new System.Windows.Forms.ComboBox();
            this.btnRefreshPorts = new System.Windows.Forms.Button();
            this.btnConnect = new System.Windows.Forms.Button();

            this.grpNetwork = new System.Windows.Forms.GroupBox();
            this.lblIPAddress = new System.Windows.Forms.Label();
            this.txtIPAddress = new System.Windows.Forms.TextBox();
            this.lblIPError = new System.Windows.Forms.Label();
            this.lblSubnetMask = new System.Windows.Forms.Label();
            this.txtSubnetMask = new System.Windows.Forms.TextBox();
            this.lblGateway = new System.Windows.Forms.Label();
            this.txtGateway = new System.Windows.Forms.TextBox();
            this.btnSetNetwork = new System.Windows.Forms.Button();

            // Relay tab controls
            this.grpRelay1 = new System.Windows.Forms.GroupBox();
            this.lblTrigger1 = new System.Windows.Forms.Label();
            this.textBoxTrigger = new System.Windows.Forms.TextBox();
            this.lblTrigger1Error = new System.Windows.Forms.Label();
            this.btnSetTrigger = new System.Windows.Forms.Button();
            this.btnTestRelay1 = new System.Windows.Forms.Button();
            this.lblRelay1Status = new System.Windows.Forms.Label();
            this.pnlRelay1Indicator = new System.Windows.Forms.Panel();

            this.grpRelay2 = new System.Windows.Forms.GroupBox();
            this.lblTrigger2 = new System.Windows.Forms.Label();
            this.textBoxTrigger2 = new System.Windows.Forms.TextBox();
            this.lblTrigger2Error = new System.Windows.Forms.Label();
            this.btnSetTrigger2 = new System.Windows.Forms.Button();
            this.btnTestRelay2 = new System.Windows.Forms.Button();
            this.lblRelay2Status = new System.Windows.Forms.Label();
            this.pnlRelay2Indicator = new System.Windows.Forms.Panel();

            // Log tab controls
            this.txtDebug = new System.Windows.Forms.TextBox();
            this.btnClearLog = new System.Windows.Forms.Button();

            this.tabControl.SuspendLayout();
            this.tabConnection.SuspendLayout();
            this.tabRelays.SuspendLayout();
            this.tabLog.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.grpSerialPort.SuspendLayout();
            this.grpNetwork.SuspendLayout();
            this.grpRelay1.SuspendLayout();
            this.grpRelay2.SuspendLayout();
            this.SuspendLayout();

            // =====================
            // TAB CONTROL
            // =====================
            this.tabControl.Controls.Add(this.tabConnection);
            this.tabControl.Controls.Add(this.tabRelays);
            this.tabControl.Controls.Add(this.tabLog);
            this.tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl.Location = new System.Drawing.Point(0, 0);
            this.tabControl.Name = "tabControl";
            this.tabControl.Padding = new System.Drawing.Point(12, 6);
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(484, 381);
            this.tabControl.TabIndex = 0;

            // =====================
            // CONNECTION TAB
            // =====================
            this.tabConnection.Controls.Add(this.grpSerialPort);
            this.tabConnection.Controls.Add(this.grpNetwork);
            this.tabConnection.Location = new System.Drawing.Point(4, 28);
            this.tabConnection.Name = "tabConnection";
            this.tabConnection.Padding = new System.Windows.Forms.Padding(12);
            this.tabConnection.Size = new System.Drawing.Size(476, 349);
            this.tabConnection.TabIndex = 0;
            this.tabConnection.Text = "Connection";
            this.tabConnection.UseVisualStyleBackColor = true;

            // --- Serial Port Group ---
            this.grpSerialPort.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) | System.Windows.Forms.AnchorStyles.Right)));
            this.grpSerialPort.Controls.Add(this.lblSerialPort);
            this.grpSerialPort.Controls.Add(this.cmbCOMPorts);
            this.grpSerialPort.Controls.Add(this.btnRefreshPorts);
            this.grpSerialPort.Controls.Add(this.btnConnect);
            this.grpSerialPort.Controls.Add(this.btnAbout);
            this.grpSerialPort.Location = new System.Drawing.Point(15, 15);
            this.grpSerialPort.Name = "grpSerialPort";
            this.grpSerialPort.Size = new System.Drawing.Size(446, 90);
            this.grpSerialPort.TabIndex = 0;
            this.grpSerialPort.TabStop = false;
            this.grpSerialPort.Text = "Serial Port";

            this.lblSerialPort.AutoSize = true;
            this.lblSerialPort.Location = new System.Drawing.Point(15, 32);
            this.lblSerialPort.Name = "lblSerialPort";
            this.lblSerialPort.Size = new System.Drawing.Size(29, 13);
            this.lblSerialPort.Text = "Port:";

            this.cmbCOMPorts.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbCOMPorts.Location = new System.Drawing.Point(50, 28);
            this.cmbCOMPorts.Name = "cmbCOMPorts";
            this.cmbCOMPorts.Size = new System.Drawing.Size(100, 21);
            this.cmbCOMPorts.TabIndex = 1;

            this.btnRefreshPorts.Location = new System.Drawing.Point(158, 26);
            this.btnRefreshPorts.Name = "btnRefreshPorts";
            this.btnRefreshPorts.Size = new System.Drawing.Size(26, 25);
            this.btnRefreshPorts.TabIndex = 2;
            this.btnRefreshPorts.Text = "\u21BB";
            this.btnRefreshPorts.UseVisualStyleBackColor = true;
            this.btnRefreshPorts.Click += new System.EventHandler(this.btnRefreshPorts_Click);

            this.btnConnect.Location = new System.Drawing.Point(200, 26);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(120, 25);
            this.btnConnect.TabIndex = 3;
            this.btnConnect.Text = "Connect";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);

            this.btnAbout = new System.Windows.Forms.Button();
            this.btnAbout.Location = new System.Drawing.Point(340, 26);
            this.btnAbout.Name = "btnAbout";
            this.btnAbout.Size = new System.Drawing.Size(90, 25);
            this.btnAbout.TabIndex = 4;
            this.btnAbout.Text = "About";
            this.btnAbout.UseVisualStyleBackColor = true;
            this.btnAbout.Click += new System.EventHandler(this.btnAbout_Click);

            // --- Network Config Group ---
            this.grpNetwork.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) | System.Windows.Forms.AnchorStyles.Right)));
            this.grpNetwork.Controls.Add(this.lblIPAddress);
            this.grpNetwork.Controls.Add(this.txtIPAddress);
            this.grpNetwork.Controls.Add(this.lblIPError);
            this.grpNetwork.Controls.Add(this.lblSubnetMask);
            this.grpNetwork.Controls.Add(this.txtSubnetMask);
            this.grpNetwork.Controls.Add(this.lblGateway);
            this.grpNetwork.Controls.Add(this.txtGateway);
            this.grpNetwork.Controls.Add(this.btnSetNetwork);
            this.grpNetwork.Enabled = false;
            this.grpNetwork.Location = new System.Drawing.Point(15, 115);
            this.grpNetwork.Name = "grpNetwork";
            this.grpNetwork.Size = new System.Drawing.Size(446, 160);
            this.grpNetwork.TabIndex = 1;
            this.grpNetwork.TabStop = false;
            this.grpNetwork.Text = "Network Configuration";

            this.lblIPAddress.AutoSize = true;
            this.lblIPAddress.Location = new System.Drawing.Point(15, 30);
            this.lblIPAddress.Name = "lblIPAddress";
            this.lblIPAddress.Size = new System.Drawing.Size(64, 13);
            this.lblIPAddress.Text = "IP Address:";

            this.txtIPAddress.Location = new System.Drawing.Point(120, 27);
            this.txtIPAddress.Name = "txtIPAddress";
            this.txtIPAddress.Size = new System.Drawing.Size(150, 20);
            this.txtIPAddress.TabIndex = 1;

            this.lblIPError.AutoSize = true;
            this.lblIPError.ForeColor = System.Drawing.Color.Red;
            this.lblIPError.Location = new System.Drawing.Point(276, 30);
            this.lblIPError.Name = "lblIPError";
            this.lblIPError.Size = new System.Drawing.Size(0, 13);
            this.lblIPError.Text = "";

            this.lblSubnetMask.AutoSize = true;
            this.lblSubnetMask.Location = new System.Drawing.Point(15, 60);
            this.lblSubnetMask.Name = "lblSubnetMask";
            this.lblSubnetMask.Size = new System.Drawing.Size(76, 13);
            this.lblSubnetMask.Text = "Subnet Mask:";

            this.txtSubnetMask.Location = new System.Drawing.Point(120, 57);
            this.txtSubnetMask.Name = "txtSubnetMask";
            this.txtSubnetMask.Size = new System.Drawing.Size(150, 20);
            this.txtSubnetMask.TabIndex = 2;
            this.txtSubnetMask.Text = "255.255.255.0";

            this.lblGateway.AutoSize = true;
            this.lblGateway.Location = new System.Drawing.Point(15, 90);
            this.lblGateway.Name = "lblGateway";
            this.lblGateway.Size = new System.Drawing.Size(52, 13);
            this.lblGateway.Text = "Gateway:";

            this.txtGateway.Location = new System.Drawing.Point(120, 87);
            this.txtGateway.Name = "txtGateway";
            this.txtGateway.Size = new System.Drawing.Size(150, 20);
            this.txtGateway.TabIndex = 3;

            this.btnSetNetwork.Location = new System.Drawing.Point(120, 120);
            this.btnSetNetwork.Name = "btnSetNetwork";
            this.btnSetNetwork.Size = new System.Drawing.Size(150, 28);
            this.btnSetNetwork.TabIndex = 4;
            this.btnSetNetwork.Text = "Apply Network Settings";
            this.btnSetNetwork.UseVisualStyleBackColor = true;
            this.btnSetNetwork.Click += new System.EventHandler(this.btnSetIP_Click);

            // =====================
            // RELAYS TAB
            // =====================
            this.tabRelays.Controls.Add(this.grpRelay1);
            this.tabRelays.Controls.Add(this.grpRelay2);
            this.tabRelays.Location = new System.Drawing.Point(4, 28);
            this.tabRelays.Name = "tabRelays";
            this.tabRelays.Padding = new System.Windows.Forms.Padding(12);
            this.tabRelays.Size = new System.Drawing.Size(476, 349);
            this.tabRelays.TabIndex = 1;
            this.tabRelays.Text = "Relay Configuration";
            this.tabRelays.UseVisualStyleBackColor = true;

            // --- Relay 1 Group ---
            this.grpRelay1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) | System.Windows.Forms.AnchorStyles.Right)));
            this.grpRelay1.Controls.Add(this.pnlRelay1Indicator);
            this.grpRelay1.Controls.Add(this.lblRelay1Status);
            this.grpRelay1.Controls.Add(this.lblTrigger1);
            this.grpRelay1.Controls.Add(this.textBoxTrigger);
            this.grpRelay1.Controls.Add(this.lblTrigger1Error);
            this.grpRelay1.Controls.Add(this.btnSetTrigger);
            this.grpRelay1.Controls.Add(this.btnTestRelay1);
            this.grpRelay1.Enabled = false;
            this.grpRelay1.Location = new System.Drawing.Point(15, 15);
            this.grpRelay1.Name = "grpRelay1";
            this.grpRelay1.Size = new System.Drawing.Size(446, 130);
            this.grpRelay1.TabIndex = 0;
            this.grpRelay1.TabStop = false;
            this.grpRelay1.Text = "Relay 1";

            this.pnlRelay1Indicator.BackColor = System.Drawing.Color.Gray;
            this.pnlRelay1Indicator.Location = new System.Drawing.Point(15, 25);
            this.pnlRelay1Indicator.Name = "pnlRelay1Indicator";
            this.pnlRelay1Indicator.Size = new System.Drawing.Size(16, 16);
            this.pnlRelay1Indicator.TabIndex = 0;

            this.lblRelay1Status.AutoSize = true;
            this.lblRelay1Status.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold);
            this.lblRelay1Status.Location = new System.Drawing.Point(37, 27);
            this.lblRelay1Status.Name = "lblRelay1Status";
            this.lblRelay1Status.Size = new System.Drawing.Size(28, 13);
            this.lblRelay1Status.Text = "OFF";

            this.lblTrigger1.AutoSize = true;
            this.lblTrigger1.Location = new System.Drawing.Point(15, 60);
            this.lblTrigger1.Name = "lblTrigger1";
            this.lblTrigger1.Size = new System.Drawing.Size(89, 13);
            this.lblTrigger1.Text = "Trigger message:";

            this.textBoxTrigger.Location = new System.Drawing.Point(120, 57);
            this.textBoxTrigger.Name = "textBoxTrigger";
            this.textBoxTrigger.Size = new System.Drawing.Size(200, 20);
            this.textBoxTrigger.TabIndex = 1;
            this.textBoxTrigger.MaxLength = 79;

            this.lblTrigger1Error.AutoSize = true;
            this.lblTrigger1Error.ForeColor = System.Drawing.Color.Red;
            this.lblTrigger1Error.Location = new System.Drawing.Point(120, 80);
            this.lblTrigger1Error.Name = "lblTrigger1Error";
            this.lblTrigger1Error.Size = new System.Drawing.Size(0, 13);
            this.lblTrigger1Error.Text = "";

            this.btnSetTrigger.Location = new System.Drawing.Point(120, 95);
            this.btnSetTrigger.Name = "btnSetTrigger";
            this.btnSetTrigger.Size = new System.Drawing.Size(120, 25);
            this.btnSetTrigger.TabIndex = 2;
            this.btnSetTrigger.Text = "Apply Trigger";
            this.btnSetTrigger.UseVisualStyleBackColor = true;
            this.btnSetTrigger.Click += new System.EventHandler(this.btnSetTrigger_Click);

            this.btnTestRelay1.Location = new System.Drawing.Point(250, 95);
            this.btnTestRelay1.Name = "btnTestRelay1";
            this.btnTestRelay1.Size = new System.Drawing.Size(100, 25);
            this.btnTestRelay1.TabIndex = 3;
            this.btnTestRelay1.Text = "Test Relay";
            this.btnTestRelay1.UseVisualStyleBackColor = true;
            this.btnTestRelay1.Click += new System.EventHandler(this.btnTestRelay1_Click);

            // --- Relay 2 Group ---
            this.grpRelay2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) | System.Windows.Forms.AnchorStyles.Right)));
            this.grpRelay2.Controls.Add(this.pnlRelay2Indicator);
            this.grpRelay2.Controls.Add(this.lblRelay2Status);
            this.grpRelay2.Controls.Add(this.lblTrigger2);
            this.grpRelay2.Controls.Add(this.textBoxTrigger2);
            this.grpRelay2.Controls.Add(this.lblTrigger2Error);
            this.grpRelay2.Controls.Add(this.btnSetTrigger2);
            this.grpRelay2.Controls.Add(this.btnTestRelay2);
            this.grpRelay2.Enabled = false;
            this.grpRelay2.Location = new System.Drawing.Point(15, 155);
            this.grpRelay2.Name = "grpRelay2";
            this.grpRelay2.Size = new System.Drawing.Size(446, 130);
            this.grpRelay2.TabIndex = 1;
            this.grpRelay2.TabStop = false;
            this.grpRelay2.Text = "Relay 2";

            this.pnlRelay2Indicator.BackColor = System.Drawing.Color.Gray;
            this.pnlRelay2Indicator.Location = new System.Drawing.Point(15, 25);
            this.pnlRelay2Indicator.Name = "pnlRelay2Indicator";
            this.pnlRelay2Indicator.Size = new System.Drawing.Size(16, 16);
            this.pnlRelay2Indicator.TabIndex = 0;

            this.lblRelay2Status.AutoSize = true;
            this.lblRelay2Status.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold);
            this.lblRelay2Status.Location = new System.Drawing.Point(37, 27);
            this.lblRelay2Status.Name = "lblRelay2Status";
            this.lblRelay2Status.Size = new System.Drawing.Size(28, 13);
            this.lblRelay2Status.Text = "OFF";

            this.lblTrigger2.AutoSize = true;
            this.lblTrigger2.Location = new System.Drawing.Point(15, 60);
            this.lblTrigger2.Name = "lblTrigger2";
            this.lblTrigger2.Size = new System.Drawing.Size(89, 13);
            this.lblTrigger2.Text = "Trigger message:";

            this.textBoxTrigger2.Location = new System.Drawing.Point(120, 57);
            this.textBoxTrigger2.Name = "textBoxTrigger2";
            this.textBoxTrigger2.Size = new System.Drawing.Size(200, 20);
            this.textBoxTrigger2.TabIndex = 1;
            this.textBoxTrigger2.MaxLength = 79;

            this.lblTrigger2Error.AutoSize = true;
            this.lblTrigger2Error.ForeColor = System.Drawing.Color.Red;
            this.lblTrigger2Error.Location = new System.Drawing.Point(120, 80);
            this.lblTrigger2Error.Name = "lblTrigger2Error";
            this.lblTrigger2Error.Size = new System.Drawing.Size(0, 13);
            this.lblTrigger2Error.Text = "";

            this.btnSetTrigger2.Location = new System.Drawing.Point(120, 95);
            this.btnSetTrigger2.Name = "btnSetTrigger2";
            this.btnSetTrigger2.Size = new System.Drawing.Size(120, 25);
            this.btnSetTrigger2.TabIndex = 2;
            this.btnSetTrigger2.Text = "Apply Trigger";
            this.btnSetTrigger2.UseVisualStyleBackColor = true;
            this.btnSetTrigger2.Click += new System.EventHandler(this.btnSetTrigger2_Click);

            this.btnTestRelay2.Location = new System.Drawing.Point(250, 95);
            this.btnTestRelay2.Name = "btnTestRelay2";
            this.btnTestRelay2.Size = new System.Drawing.Size(100, 25);
            this.btnTestRelay2.TabIndex = 3;
            this.btnTestRelay2.Text = "Test Relay";
            this.btnTestRelay2.UseVisualStyleBackColor = true;
            this.btnTestRelay2.Click += new System.EventHandler(this.btnTestRelay2_Click);

            // =====================
            // LOG TAB
            // =====================
            this.tabLog.Controls.Add(this.txtDebug);
            this.tabLog.Controls.Add(this.btnClearLog);
            this.tabLog.Location = new System.Drawing.Point(4, 28);
            this.tabLog.Name = "tabLog";
            this.tabLog.Padding = new System.Windows.Forms.Padding(12);
            this.tabLog.Size = new System.Drawing.Size(476, 349);
            this.tabLog.TabIndex = 2;
            this.tabLog.Text = "Event Log";
            this.tabLog.UseVisualStyleBackColor = true;

            this.txtDebug.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) | System.Windows.Forms.AnchorStyles.Left) | System.Windows.Forms.AnchorStyles.Right)));
            this.txtDebug.BackColor = System.Drawing.Color.FromArgb(30, 30, 30);
            this.txtDebug.Font = new System.Drawing.Font("Consolas", 9F);
            this.txtDebug.ForeColor = System.Drawing.Color.FromArgb(220, 220, 220);
            this.txtDebug.Location = new System.Drawing.Point(15, 15);
            this.txtDebug.Multiline = true;
            this.txtDebug.Name = "txtDebug";
            this.txtDebug.ReadOnly = true;
            this.txtDebug.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.txtDebug.Size = new System.Drawing.Size(446, 289);
            this.txtDebug.TabIndex = 0;
            this.txtDebug.WordWrap = false;

            this.btnClearLog.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnClearLog.Location = new System.Drawing.Point(376, 312);
            this.btnClearLog.Name = "btnClearLog";
            this.btnClearLog.Size = new System.Drawing.Size(85, 25);
            this.btnClearLog.TabIndex = 1;
            this.btnClearLog.Text = "Clear Log";
            this.btnClearLog.UseVisualStyleBackColor = true;
            this.btnClearLog.Click += new System.EventHandler(this.btnClearLog_Click);

            // =====================
            // STATUS STRIP
            // =====================
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
                this.statusConnection,
                this.statusCOMPort,
                this.statusBoardIP,
                this.statusFirmware
            });
            this.statusStrip.Location = new System.Drawing.Point(0, 381);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Size = new System.Drawing.Size(484, 22);
            this.statusStrip.TabIndex = 1;

            this.statusConnection.AutoSize = false;
            this.statusConnection.BackColor = System.Drawing.Color.FromArgb(180, 0, 0);
            this.statusConnection.ForeColor = System.Drawing.Color.White;
            this.statusConnection.Name = "statusConnection";
            this.statusConnection.Size = new System.Drawing.Size(90, 17);
            this.statusConnection.Text = " Disconnected";
            this.statusConnection.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;

            this.statusCOMPort.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Left;
            this.statusCOMPort.Name = "statusCOMPort";
            this.statusCOMPort.Size = new System.Drawing.Size(30, 17);
            this.statusCOMPort.Text = "";

            this.statusBoardIP.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Left;
            this.statusBoardIP.Name = "statusBoardIP";
            this.statusBoardIP.Size = new System.Drawing.Size(30, 17);
            this.statusBoardIP.Text = "";

            this.statusFirmware.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Left;
            this.statusFirmware.Name = "statusFirmware";
            this.statusFirmware.Spring = true;
            this.statusFirmware.Size = new System.Drawing.Size(280, 17);
            this.statusFirmware.Text = "App v0.3.0";
            this.statusFirmware.TextAlign = System.Drawing.ContentAlignment.MiddleRight;

            // =====================
            // FORM
            // =====================
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 403);
            this.Controls.Add(this.tabControl);
            this.Controls.Add(this.statusStrip);
            this.MinimumSize = new System.Drawing.Size(500, 440);
            this.Name = "Form1";
            this.Text = "IDS Network Monitor";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.tabControl.ResumeLayout(false);
            this.tabConnection.ResumeLayout(false);
            this.tabRelays.ResumeLayout(false);
            this.tabLog.ResumeLayout(false);
            this.tabLog.PerformLayout();
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.grpSerialPort.ResumeLayout(false);
            this.grpSerialPort.PerformLayout();
            this.grpNetwork.ResumeLayout(false);
            this.grpNetwork.PerformLayout();
            this.grpRelay1.ResumeLayout(false);
            this.grpRelay1.PerformLayout();
            this.grpRelay2.ResumeLayout(false);
            this.grpRelay2.PerformLayout();
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

        private System.Windows.Forms.GroupBox grpNetwork;
        private System.Windows.Forms.Label lblIPAddress;
        private System.Windows.Forms.TextBox txtIPAddress;
        private System.Windows.Forms.Label lblIPError;
        private System.Windows.Forms.Label lblSubnetMask;
        private System.Windows.Forms.TextBox txtSubnetMask;
        private System.Windows.Forms.Label lblGateway;
        private System.Windows.Forms.TextBox txtGateway;
        private System.Windows.Forms.Button btnSetNetwork;

        // Relay tab
        private System.Windows.Forms.GroupBox grpRelay1;
        private System.Windows.Forms.Label lblTrigger1;
        private System.Windows.Forms.TextBox textBoxTrigger;
        private System.Windows.Forms.Label lblTrigger1Error;
        private System.Windows.Forms.Button btnSetTrigger;
        private System.Windows.Forms.Button btnTestRelay1;
        private System.Windows.Forms.Label lblRelay1Status;
        private System.Windows.Forms.Panel pnlRelay1Indicator;

        private System.Windows.Forms.GroupBox grpRelay2;
        private System.Windows.Forms.Label lblTrigger2;
        private System.Windows.Forms.TextBox textBoxTrigger2;
        private System.Windows.Forms.Label lblTrigger2Error;
        private System.Windows.Forms.Button btnSetTrigger2;
        private System.Windows.Forms.Button btnTestRelay2;
        private System.Windows.Forms.Label lblRelay2Status;
        private System.Windows.Forms.Panel pnlRelay2Indicator;

        // Connection tab
        private System.Windows.Forms.Button btnAbout;

        // Log tab
        private System.Windows.Forms.TextBox txtDebug;
        private System.Windows.Forms.Button btnClearLog;
    }
}
