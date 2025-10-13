using ConvMVVM2.WPF.Behaviors.Base;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace UITest.Behaviors
{
    public class SizeChangedBehavior : Behavior<FrameworkElement>
    {
        #region Public Functions
        protected override void OnAttached()
        {
            this.AssociatedObject.SizeChanged += AssociatedObject_SizeChanged;

        }

        protected override void OnDetaching()
        {
            this.AssociatedObject.SizeChanged -= AssociatedObject_SizeChanged;
        }
        #endregion

        #region Dependency Property
        public static DependencyProperty CurrentSizeProperty = DependencyProperty.Register("CurrentSize", typeof(Size), typeof(SizeChangedBehavior));
        public Size CurrentSize
        {
            get => (Size)GetValue(CurrentSizeProperty);
            set => SetValue(CurrentSizeProperty, value);
        }
        #endregion

        #region Event Handler
        private void AssociatedObject_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            this.CurrentSize = e.NewSize;
        }
        #endregion
    }
}
