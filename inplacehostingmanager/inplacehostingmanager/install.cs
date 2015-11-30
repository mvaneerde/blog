using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Deployment.Application;

namespace install
{
    class install
    {
        static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Expected one argument, not {0}", args.Length);
                return;
            }

            new install().InstallApplication(new Uri(args[0]));
        }

        private void InstallApplication(Uri app)
        {
            var manager = new InPlaceHostingManager(app, false);
            manager.GetManifestCompleted +=
                new EventHandler<GetManifestCompletedEventArgs>(
                    OnGetManifestCompleted
                );

            // I would love to just "await manager.GetManifestAsync" here
            // but GetManifestAsync returns void
            _event = new AutoResetEvent(false);
            // Console.WriteLine("Getting manifest for {0}", app);
            manager.GetManifestAsync();
            _event.WaitOne();

            // this call is synchronous
            // Console.WriteLine("Asserting application requirements for {0}", app);
            manager.AssertApplicationRequirements(true);
            // Console.WriteLine("AssertApplicationRequirements passes");

            manager.DownloadApplicationCompleted +=
                new EventHandler<DownloadApplicationCompletedEventArgs>(
                    OnDownloadApplicationCompleted
                );
            // Console.WriteLine("Downloading and installing {0}", app);
            manager.DownloadApplicationAsync();
            _event.WaitOne();
        }

        private static void OnGetManifestCompleted(object sender, GetManifestCompletedEventArgs args)
        {
            // Console.WriteLine("OnGetManifestCompleted called");
            _event.Set();
        }

        private static void OnDownloadApplicationCompleted(object sender, DownloadApplicationCompletedEventArgs args)
        {
            // Console.WriteLine("OnDownloadApplicationCompleted called");
            _event.Set();
        }

        private static AutoResetEvent _event;
    }
}
