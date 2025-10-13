using ConvMVVM2.WPF.Extensions;
using ConvMVVM2.WPF.Host;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace UITest
{
    public class Starter
    {

        [STAThread]
        public static void Main(string[] args)
        {

            var host = ConvMVVM2Host.CreateHost<BootStrapper, Application>(args, "App");
            host.AddWPFDialogService()
                .Build()
                .ShutdownMode(ShutdownMode.OnMainWindowClose)
                .RunApp<MainWindow>();
        }
    }
}
