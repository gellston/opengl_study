using ConvMVVM2.Core.Attributes;
using ConvMVVM2.Core.MVVM;
using ConvMVVM2.WPF.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Imaging;

namespace UITest.ViewModels
{
    public partial class MainViewModel : ViewModelBase, IServiceInitializable
    {
        #region Private Property
        private Hv.V1.Render render = new Hv.V1.Render();
        private System.Windows.Point startPoint = new System.Windows.Point();   
        #endregion


        #region Constructor
        public MainViewModel() { 
        
        }
        #endregion

        #region Public Property
        [Property]
        private MouseViewModel _MouseViewModel = new MouseViewModel();


        [Property]
        private WriteableBitmap _CurrentFrame = null;

        [Property]
        private Size _CurrentSize = new Size();
        #endregion

        #region Event Handler
        public void OnServiceInitialized()
        {
            this.render.OnFrameUpdateEvent += Render_OnFrameUpdateEvent;

            this.MouseViewModel.WheelEvent += MouseViewModel_WheelEvent;
            this.MouseViewModel.LeftDragEvent += MouseViewModel_LeftDragEvent;
            this.MouseViewModel.LeftDownEvent += MouseViewModel_LeftDownEvent;
            this.MouseViewModel.LeftUpEvent += MouseViewModel_LeftUpEvent;
        }

        private void Render_OnFrameUpdateEvent(Hv.V1.FrameView frame)
        {
            Application.Current.Dispatcher.Invoke(() => {
                if (this.CurrentFrame == null)
                {
                    var bitmap = BitmapImage.Create(frame.Width,
                                           frame.Height,
                                           96,
                                           96,
                                           System.Windows.Media.PixelFormats.Bgra32,
                                           null,
                                           frame.Data,
                                           frame.Stride * frame.Height,
                                           frame.Stride);

                    this.CurrentFrame = new WriteableBitmap(bitmap);

                    return;
                }

                if (this.CurrentFrame.Width != frame.Width ||
                    this.CurrentFrame.Height != frame.Height)
                {
                    var bitmap = BitmapImage.Create(frame.Width,
                                           frame.Height,
                                           96,
                                           96,
                                           System.Windows.Media.PixelFormats.Bgra32,
                                           null,
                                           frame.Data,
                                           frame.Stride * frame.Height,
                                           frame.Stride);

                    this.CurrentFrame = new WriteableBitmap(bitmap);

                    return;
                }


                this.CurrentFrame.Lock();
                this.CurrentFrame.WritePixels(new System.Windows.Int32Rect()
                {
                    Width = frame.Width,
                    Height = frame.Height,
                    X = 0,
                    Y = 0

                },
                frame.Data,
                frame.Stride * frame.Height,
                frame.Stride);
                this.CurrentFrame.Unlock();
            });
        }

        private void MouseViewModel_LeftUpEvent(System.Windows.Point point)
        {


        }

        private void MouseViewModel_LeftDownEvent(System.Windows.Point point)
        {

            this.startPoint = point;
        }

        private void MouseViewModel_LeftDragEvent(System.Windows.Point point)
        {
            if(Keyboard.IsKeyDown(Key.LeftCtrl) == false)
            {

                var delta = this.startPoint - point;
                this.render.OrbitBy((float)(delta.X * 0.5), (float)(-delta.Y * 0.5));
                this.startPoint = point;
            }
            else
            {
                var delta = this.startPoint - point;
                this.render.TranslatePixelBy((float)delta.X, (float)-delta.Y);
                this.startPoint = point;
            }



        }

        private void MouseViewModel_WheelEvent(System.Windows.Point point, bool direction)
        {

            if (direction)
            {

                this.render.ZoomBy(0.9f);
            }
            else
            {

                this.render.ZoomBy(1.1f);
            }
       
        }


        partial void OnCurrentSizeChanged(Size value)
        {
            try
            {
                var outSize = new Hv.V1.OutputDesc()
                {
                    Width = (int)value.Width,
                    Height = (int)value.Height
                };
                this.render.ResizeOutput(outSize);

            }
            catch(Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.ToString());
            }

        }
        #endregion
    }
}
