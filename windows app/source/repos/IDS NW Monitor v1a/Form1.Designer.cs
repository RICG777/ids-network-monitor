namespace IDS_NW_Monitor_v1a
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.textBoxTrigger = new System.Windows.Forms.TextBox();
            this.btnConnect = new System.Windows.Forms.Button();
            this.btnSetTrigger = new System.Windows.Forms.Button();
            this.lblConnectionStatus = new System.Windows.Forms.Label();
            this.txtDebug = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.txtIPAddress = new System.Windows.Forms.TextBox();
            this.btnSetIP = new System.Windows.Forms.Button();
            this.btnSetTrigger2 = new System.Windows.Forms.Button();
            this.textBoxTrigger2 = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.cmbCOMPorts = new System.Windows.Forms.ComboBox();
            this.buildVersion = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(41, 180);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(150, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Enter Relay1 Trigger Message";
            // 
            // textBoxTrigger
            // 
            this.textBoxTrigger.Location = new System.Drawing.Point(44, 199);
            this.textBoxTrigger.Name = "textBoxTrigger";
            this.textBoxTrigger.Size = new System.Drawing.Size(119, 20);
            this.textBoxTrigger.TabIndex = 1;
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(133, 61);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(106, 31);
            this.btnConnect.TabIndex = 2;
            this.btnConnect.Text = "Connect To Board";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // btnSetTrigger
            // 
            this.btnSetTrigger.Location = new System.Drawing.Point(169, 196);
            this.btnSetTrigger.Name = "btnSetTrigger";
            this.btnSetTrigger.Size = new System.Drawing.Size(106, 23);
            this.btnSetTrigger.TabIndex = 3;
            this.btnSetTrigger.Text = "Send Trigger Message";
            this.btnSetTrigger.UseVisualStyleBackColor = true;
            this.btnSetTrigger.Click += new System.EventHandler(this.btnSetTrigger_Click);
            // 
            // lblConnectionStatus
            // 
            this.lblConnectionStatus.AutoSize = true;
            this.lblConnectionStatus.Location = new System.Drawing.Point(130, 95);
            this.lblConnectionStatus.Name = "lblConnectionStatus";
            this.lblConnectionStatus.Size = new System.Drawing.Size(109, 13);
            this.lblConnectionStatus.TabIndex = 4;
            this.lblConnectionStatus.Text = "Status: Disconnected";
            // 
            // txtDebug
            // 
            this.txtDebug.Location = new System.Drawing.Point(35, 305);
            this.txtDebug.Multiline = true;
            this.txtDebug.Name = "txtDebug";
            this.txtDebug.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtDebug.Size = new System.Drawing.Size(324, 124);
            this.txtDebug.TabIndex = 5;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(153, 289);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(80, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "Debug Console";
            this.label2.Click += new System.EventHandler(this.label2_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(41, 121);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(77, 13);
            this.label3.TabIndex = 7;
            this.label3.Text = "Set IP Address";
            // 
            // txtIPAddress
            // 
            this.txtIPAddress.Location = new System.Drawing.Point(44, 137);
            this.txtIPAddress.Name = "txtIPAddress";
            this.txtIPAddress.Size = new System.Drawing.Size(119, 20);
            this.txtIPAddress.TabIndex = 8;
            // 
            // btnSetIP
            // 
            this.btnSetIP.Location = new System.Drawing.Point(170, 133);
            this.btnSetIP.Name = "btnSetIP";
            this.btnSetIP.Size = new System.Drawing.Size(105, 23);
            this.btnSetIP.TabIndex = 9;
            this.btnSetIP.Text = "Send IP Address";
            this.btnSetIP.UseVisualStyleBackColor = true;
            this.btnSetIP.Click += new System.EventHandler(this.btnSetIP_Click);
            // 
            // btnSetTrigger2
            // 
            this.btnSetTrigger2.Location = new System.Drawing.Point(169, 249);
            this.btnSetTrigger2.Name = "btnSetTrigger2";
            this.btnSetTrigger2.Size = new System.Drawing.Size(106, 23);
            this.btnSetTrigger2.TabIndex = 12;
            this.btnSetTrigger2.Text = "Send Trigger Message";
            this.btnSetTrigger2.UseVisualStyleBackColor = true;
            this.btnSetTrigger2.Click += new System.EventHandler(this.btnSetTrigger2_Click);
            // 
            // textBoxTrigger2
            // 
            this.textBoxTrigger2.Location = new System.Drawing.Point(44, 252);
            this.textBoxTrigger2.Name = "textBoxTrigger2";
            this.textBoxTrigger2.Size = new System.Drawing.Size(119, 20);
            this.textBoxTrigger2.TabIndex = 11;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(41, 233);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(150, 13);
            this.label4.TabIndex = 10;
            this.label4.Text = "Enter Relay2 Trigger Message";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(41, 18);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(74, 13);
            this.label5.TabIndex = 13;
            this.label5.Text = "Set Serial Port";
            // 
            // cmbCOMPorts
            // 
            this.cmbCOMPorts.FormattingEnabled = true;
            this.cmbCOMPorts.Location = new System.Drawing.Point(44, 34);
            this.cmbCOMPorts.Name = "cmbCOMPorts";
            this.cmbCOMPorts.Size = new System.Drawing.Size(121, 21);
            this.cmbCOMPorts.TabIndex = 14;
            // 
            // buildVersion
            // 
            this.buildVersion.AutoSize = true;
            this.buildVersion.Location = new System.Drawing.Point(155, 443);
            this.buildVersion.Name = "buildVersion";
            this.buildVersion.Size = new System.Drawing.Size(71, 13);
            this.buildVersion.TabIndex = 15;
            this.buildVersion.Text = "Build Version:";
            this.buildVersion.Click += new System.EventHandler(this.buildVersion_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(381, 468);
            this.Controls.Add(this.buildVersion);
            this.Controls.Add(this.cmbCOMPorts);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.btnSetTrigger2);
            this.Controls.Add(this.textBoxTrigger2);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.btnSetIP);
            this.Controls.Add(this.txtIPAddress);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtDebug);
            this.Controls.Add(this.lblConnectionStatus);
            this.Controls.Add(this.btnSetTrigger);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.textBoxTrigger);
            this.Controls.Add(this.label1);
            this.Name = "Form1";
            this.Text = "IDS Network Monitor";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textBoxTrigger;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.Button btnSetTrigger;
        private System.Windows.Forms.Label lblConnectionStatus;
        private System.Windows.Forms.TextBox txtDebug;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtIPAddress;
        private System.Windows.Forms.Button btnSetIP;
        private System.Windows.Forms.Button btnSetTrigger2;
        private System.Windows.Forms.TextBox textBoxTrigger2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox cmbCOMPorts;
        private System.Windows.Forms.Label buildVersion;
    }
}

