using System.Diagnostics;
using System.IO;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using IronPython.Hosting;
using IronPython.Runtime;
using Microsoft.Scripting;
using Microsoft.Scripting.Hosting;
using System.Text.RegularExpressions;

namespace UsedCarPriceEstimate
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

            comboBox1.Items.Add("Hatchback");
            comboBox1.Items.Add("Van");
            comboBox1.Items.Add("Sedan");
            comboBox1.Items.Add("Ute");
            comboBox1.Items.Add("Station Wagon");
            comboBox1.Items.Add("RV/SUV");
            comboBox1.Items.Add("Convertible");
            comboBox1.Items.Add("Coupe");
            comboBox1.Items.Add("Other");
            comboBox1.SelectedIndex = comboBox1.FindStringExact("Hatchback");

            comboBox2.Items.Add("manual");
            comboBox2.Items.Add("automatic");
            comboBox2.Items.Add("tiptronic");
            comboBox2.SelectedIndex = comboBox2.FindStringExact("manual");

            comboBox3.Items.Add("petrol");
            comboBox3.Items.Add("diesel");
            comboBox3.Items.Add("hybrid");
            comboBox3.Items.Add("electric");
            comboBox3.SelectedIndex = comboBox3.FindStringExact("petrol");
        }

        private string GetPredictions()
        {
            string arg = Path.Combine(Directory.GetCurrentDirectory(), "result.txt");
            string predictions = File.ReadAllText(arg);
            return predictions;

        }

        private string AddCommasIfRequired(string path)
        {
            return (path.Contains(" ")) ? "\"" + path + "\"" : path;
        }

        private void PythonRun(string cmd, string args)
        {
            // Run python script without displaying cosole window
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.FileName = cmd;
            startInfo.Arguments = args;
            startInfo.RedirectStandardOutput = true;
            startInfo.RedirectStandardError = true;
            startInfo.UseShellExecute = false;
            startInfo.CreateNoWindow = true;
            Process p = new Process();
            p.StartInfo = startInfo;
            p.EnableRaisingEvents = true;
            p.Start();
            p.WaitForExit();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string km = textBox1.Text;
            string year = textBox3.Text;
            string displacement = textBox4.Text;
            string car_type = comboBox1.Text;
            string transmission = comboBox2.Text;
            string fuel_type = comboBox3.Text;
            if(km == "" || year == "" || displacement == "" || car_type == "" || transmission == "" || fuel_type == "")
            {
                textBox2.Text = "please fill blanks";
                return;
            }

            string arg = Path.Combine(Directory.GetCurrentDirectory(), "predict.py");
            string py_arg = string.Format("\"{0}\" \"{1}\" \"{2}\" \"{3}\" \"{4}\" \"{5}\" \"{6}\"", arg, car_type, km, year, displacement, fuel_type, transmission);
            textBox2.ForeColor = Color.Red;
            textBox2.Text = "Estimating...";
            PythonRun("python.exe", py_arg);
            string result = GetPredictions();
            // string dd = py.main(textBox1.Lines);//调用脚本文件中对应的函数
            textBox2.Text = result;
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            var reg = new Regex("^[0-9]*$");
            var str = textBox1.Text.Trim();
            var sb = new StringBuilder();
            if (!reg.IsMatch(str))
            {
                for (int i = 0; i < str.Length; i++)
                {
                    if (reg.IsMatch(str[i].ToString()))
                    {
                        sb.Append(str[i].ToString());
                    }
                }
                textBox1.Text = sb.ToString();
                textBox1.SelectionStart = textBox1.Text.Length;
            }
        }

        private void textBox3_TextChanged(object sender, EventArgs e)
        {
            var reg = new Regex("^[0-9]*$");
            var str = textBox3.Text.Trim();
            var sb = new StringBuilder();
            if (!reg.IsMatch(str))
            {
                for (int i = 0; i < str.Length; i++)
                {
                    if (reg.IsMatch(str[i].ToString()))
                    {
                        sb.Append(str[i].ToString());
                    }
                }
                textBox3.Text = sb.ToString();
                textBox3.SelectionStart = textBox3.Text.Length;
            }
        }

        private void textBox4_TextChanged(object sender, EventArgs e)
        {
            var reg = new Regex("^[0-9]*$");
            var str = textBox4.Text.Trim();
            var sb = new StringBuilder();
            if (!reg.IsMatch(str))
            {
                for (int i = 0; i < str.Length; i++)
                {
                    if (reg.IsMatch(str[i].ToString()))
                    {
                        sb.Append(str[i].ToString());
                    }
                }
                textBox4.Text = sb.ToString();
                textBox4.SelectionStart = textBox4.Text.Length;
            }
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void comboBox3_SelectedIndexChanged(object sender, EventArgs e)
        {

        }
    }
}
