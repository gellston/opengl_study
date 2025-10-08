using Hv.V1;

namespace ConsoleTest
{
    internal class Program
    {
        static void Main(string[] args)
        {
            

            Hv.V1.Render render = new Hv.V1.Render();

            render.OnFrameUpdateEvent += Render_OnFrameUpdateEvent;

            Hv.V1.OutputDesc output = new Hv.V1.OutputDesc();
            output.Height = 600;
            output.Width = 800;

            render.ResizeOutput(output);

            while (true)
            {
                var key = Console.ReadKey(intercept: true);
               
                switch (key.Key)
                {
                    case ConsoleKey.Add:
                        render.ZoomBy(1.1f);
                        break;
                    case ConsoleKey.Subtract:
                        render.ZoomBy(0.9f);
                        break;
                    case ConsoleKey.DownArrow:
                        render.TranslateBy(0, -1);
                        break;
                    case ConsoleKey.UpArrow:
                        render.TranslateBy(0, 1);
                        break;
                    case ConsoleKey.LeftArrow:
                        render.TranslateBy(-1, 0);
                        break;
                    case ConsoleKey.RightArrow:
                        render.TranslateBy(1, 0);
                        break;
                    case ConsoleKey.S:
                        render.OrbitBy(0, -3);
                        break;
                    case ConsoleKey.W:
                        render.OrbitBy(0, 3);
                        break;
                    case ConsoleKey.D:
                        render.OrbitBy(3, 0);
                        break;

                    case ConsoleKey.A:
                        render.OrbitBy(-3, 0);
                        break;

                    case ConsoleKey.C:
                        return;

                    case ConsoleKey.D1:
                        Hv.V1.OutputDesc newOutput1 = new Hv.V1.OutputDesc();
                        newOutput1.Height = 256;
                        newOutput1.Width = 256;
                        render.ResizeOutput(newOutput1);
                        break;
                    case ConsoleKey.D2:
                        Hv.V1.OutputDesc newOutput2 = new Hv.V1.OutputDesc();
                        newOutput2.Height = 512;
                        newOutput2.Width = 512;
                        render.ResizeOutput(newOutput2);
                        break;
                    case ConsoleKey.D3:
                        Hv.V1.OutputDesc newOutput3 = new Hv.V1.OutputDesc();
                        newOutput3.Height = 1024;
                        newOutput3.Width = 1280;
                        render.ResizeOutput(newOutput3);
                        break;
                    default:
                        break;
                }

                Thread.Sleep(1);
            }

        }

        private static void Render_OnFrameUpdateEvent(Hv.V1.FrameView view)
        {
            var renderCvImage = OpenCvSharp.Mat.FromPixelData(view.Height, view.Width, OpenCvSharp.MatType.CV_8UC4, view.Data, view.Stride);
            OpenCvSharp.Mat cvtColor = new OpenCvSharp.Mat();

            OpenCvSharp.Cv2.CvtColor(renderCvImage, cvtColor, OpenCvSharp.ColorConversionCodes.RGBA2BGRA);


            OpenCvSharp.Cv2.NamedWindow("render result", OpenCvSharp.WindowFlags.AutoSize);
            OpenCvSharp.Cv2.ImShow("render result", cvtColor);
            OpenCvSharp.Cv2.WaitKey(1);

            renderCvImage.Dispose();
            cvtColor.Dispose();



        }
    }
}
