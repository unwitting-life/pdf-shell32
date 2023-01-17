#pragma warning disable IDE0002, IDE1006, IDE0037

using iTextSharp.text;
using iTextSharp.text.pdf;

using Newtonsoft.Json;

using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;

using WebPWrapper;

using Image = iTextSharp.text.Image;

namespace iTextSharpWrapper {
    public class impl {
        [DllImport("user32.dll")]
        public static extern int SendMessage(IntPtr hWnd, uint wMsg, IntPtr wParam, IntPtr lParam);

        public static uint WM_USER = 0x400;

        public static int invoke(string args) {
            var retVal = 0;
            try {
                retVal = invoke_(JsonConvert.DeserializeObject<args>(args));
            } catch (Exception ex) {
                Debug.WriteLine(ex);
            }
            return retVal;
        }

        // https://blog.csdn.net/lwkliuwenkang/article/details/128386143
        private static int invoke_(args args) {
            var retVal = 0;
            if (args.imageFiles.Length > 0 && !string.IsNullOrEmpty(args.pdfFilePath)) {
                var pdfFilePath = args.pdfFilePath;
                if (Path.GetFullPath(pdfFilePath) != pdfFilePath) {
                    pdfFilePath = Path.Combine(Path.GetDirectoryName(args.imageFiles[0]), Path.GetFileName(pdfFilePath));
                }
                Directory.CreateDirectory(Path.GetDirectoryName(pdfFilePath));
                var document = new Document(PageSize.A4);
                using (var pdfFileStream = new FileStream(pdfFilePath, FileMode.Create, FileAccess.Write, FileShare.None)) {
                    PdfWriter.GetInstance(document, pdfFileStream);
                    document.Open();
                    var index = 0;
                    foreach (string imageFile in args.imageFiles) {
                        try {
                            Bitmap bitmap = null;
                            Stream imageFileStream = null;
                            var extensionName = Path.GetExtension(imageFile);
                            if (extensionName.Equals(".jpeg", StringComparison.OrdinalIgnoreCase) ||
                                extensionName.Equals(".bmp", StringComparison.OrdinalIgnoreCase) ||
                                extensionName.Equals(".png", StringComparison.OrdinalIgnoreCase) ||
                                extensionName.Equals(".tiff", StringComparison.OrdinalIgnoreCase)) {
                                bitmap = new Bitmap(imageFile);
                            } else if (extensionName.Equals(".webp", StringComparison.OrdinalIgnoreCase)) {

                                // https://briancaos.wordpress.com/2022/08/29/c-convert-webp-to-jpeg-in-net/
                                bitmap = new WebP().Load(imageFile);
                            }
                            if (bitmap != null) {
                                if (bitmap.Width > bitmap.Height) {
                                    bitmap.RotateFlip(RotateFlipType.Rotate270FlipNone);
                                }
                                imageFileStream = new MemoryStream();
                                bitmap.Save(imageFileStream, ImageFormat.Png);
                                imageFileStream.Seek(0, SeekOrigin.Begin);

                                var pdfImage = Image.GetInstance(imageFileStream);
                                var imageWidth = Math.Min(pdfImage.Width, document.PageSize.Width);
                                var imageHeight = Math.Min(pdfImage.Height, document.PageSize.Height - document.BottomMargin);
                                pdfImage.ScaleToFit(imageWidth, imageHeight);
                                pdfImage.Alignment = Element.ALIGN_MIDDLE | Element.ALIGN_CENTER;
                                document.Add(pdfImage);
                                imageFileStream.Close();
                                if (args.hWnd.HasValue) {
                                    var s = Marshal.StringToHGlobalAnsi(JsonConvert.SerializeObject(new {
                                        index = index,
                                        total = args.imageFiles.Length,
                                        file = imageFile,
                                    }));
                                    impl.SendMessage(new IntPtr(args.hWnd.GetValueOrDefault()), impl.WM_USER + 1, s, IntPtr.Zero);
                                    Marshal.FreeHGlobal(s);
                                }
                                retVal++;
                            }
                            index++;
                        } catch (Exception ex) {
                            Debug.WriteLine(ex);
                        }
                    }
                    document.Close();
                    if (File.Exists(pdfFilePath)) {
                        Process.Start(new ProcessStartInfo() {
                            FileName = $"{Path.Combine(Path.GetDirectoryName(Environment.SystemDirectory), "explorer")}",
                            Arguments = $" /select,{pdfFilePath}",
                        });
                    }
                }
            }
            return retVal;
        }
    }
}
