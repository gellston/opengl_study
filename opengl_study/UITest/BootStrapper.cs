using ConvMVVM2.Core.MVVM;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UITest.ViewModels;
using UITest.Views;

namespace UITest
{
    public class BootStrapper : AppBootstrapper
    {
        protected override void OnStartUp(IServiceContainer container)
        {

        }

        protected override void RegionMapping(IRegionManager layerManager)
        {

            layerManager.Mapping<MainView>("MainView");
        }

        protected override void RegisterModules()
        {

        }

        protected override void RegisterServices(IServiceCollection serviceCollection)
        {

            //ViewModels
            serviceCollection.AddSingleton<MainViewModel>();


            //Views
            serviceCollection.AddSingleton<MainView>();


            //Windows
            serviceCollection.AddSingleton<MainWindow>();
        }

        protected override void ViewModelMapping(IViewModelMapper viewModelMapper)
        {

        }
    }
}
