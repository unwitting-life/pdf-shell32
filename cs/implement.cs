#pragma warning disable IDE0002, IDE1006, IDE0037, IDE0051

using iTextSharp.text;
using iTextSharp.text.pdf;

using Newtonsoft.Json;

using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;

using WebPWrapper;

using Image = iTextSharp.text.Image;

namespace iTextSharpWrapper {
    public class implement {
        [DllImport("user32.dll")]
        public static extern int SendMessage(IntPtr hWnd, uint wMsg, IntPtr wParam, IntPtr lParam);

        public static uint WM_USER = 0x400;

        public static int invoke(string args) {
            var retVal = 0;
            try {
                invokeAsync(JsonConvert.DeserializeObject<args>(args));
            } catch (Exception ex) {
                Debug.WriteLine(ex);
            }
            return retVal;
        }

        // https://blog.csdn.net/lwkliuwenkang/article/details/128386143
        // https://briancaos.wordpress.com/2022/08/29/c-convert-webp-to-jpeg-in-net/

        private static Task invokeAsync(args args) {
            var task = new Task(() => {
                var thread = new Thread(() => {
                    var progressWindow = new ProgressWindow() {
                        args = args,
                    };
                    progressWindow.ShowDialog();
                });
                thread.SetApartmentState(ApartmentState.STA);
                thread.Start();
                thread.Join();
            });
            task.Start();
            return task;
        }
    }
}
