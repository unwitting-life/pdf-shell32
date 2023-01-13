#pragma warning disable IDE1006

using iTextSharp.text;
using iTextSharp.text.pdf;

using Newtonsoft.Json;

using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

using WebPWrapper;

using Image = iTextSharp.text.Image;

namespace iTextSharpWrapper {
    public class impl {
        public static int invoke(string args) {
            var retVal = 0;
            try {
                retVal = invoke_(JsonConvert.DeserializeObject<args>(args));
            } catch {

            }
            return retVal;
        }

        // https://blog.csdn.net/lwkliuwenkang/article/details/128386143
        private static int invoke_(args args) {
            var retVal = 0;
            if (args.imageFiles.Length > 0) {
                var pdfFilePath = args.pdfFilePath;
                if (Path.GetFullPath(pdfFilePath) != pdfFilePath) {
                    pdfFilePath = Path.Combine(Path.GetDirectoryName(args.imageFiles[0]), Path.GetFileName(pdfFilePath));
                }
                Directory.CreateDirectory(Path.GetDirectoryName(pdfFilePath));
                var document = new Document(PageSize.A4);
                using (var pdfFileStream = new FileStream(pdfFilePath, FileMode.Create, FileAccess.Write, FileShare.None)) {
                    PdfWriter.GetInstance(document, pdfFileStream);
                    document.Open();
                    foreach (string imageFile in args.imageFiles) {
                        try {
                            Stream imageFileStream = null;
                            var extensionName = Path.GetExtension(imageFile);
                            if (extensionName.Equals(".jpeg", StringComparison.OrdinalIgnoreCase) ||
                                extensionName.Equals(".bmp", StringComparison.OrdinalIgnoreCase) ||
                                extensionName.Equals(".png", StringComparison.OrdinalIgnoreCase) ||
                                extensionName.Equals(".tiff", StringComparison.OrdinalIgnoreCase)) {
                                imageFileStream = new FileStream(imageFile, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                            } else if (extensionName.Equals(".webp", StringComparison.OrdinalIgnoreCase)) {

                                // https://briancaos.wordpress.com/2022/08/29/c-convert-webp-to-jpeg-in-net/
                                WebP webp = new WebP();
                                Bitmap bitmap = webp.Load(imageFile);
                                imageFileStream = new MemoryStream();
                                bitmap.Save(imageFileStream, ImageFormat.Png);
                                imageFileStream.Seek(0, SeekOrigin.Begin);
                            }
                            if (imageFileStream != null) {
                                Debug.WriteLine($"Converting {imageFile}...");
                                var image = Image.GetInstance(imageFileStream);
                                float height = document.Top - document.TopMargin;
                                var imageWidth = image.Width > document.Right ? document.Right : image.Width;
                                var imageHeight = image.Height > height ? height : image.Height;
                                image.ScaleToFit(imageWidth, imageHeight);
                                image.Alignment = Element.ALIGN_MIDDLE | Element.ALIGN_CENTER;
                                document.Add(image);
                                imageFileStream.Close();
                                retVal++;
                            }
                        } catch (Exception ex) {
                            Debug.WriteLine(ex);
                        }
                    }
                    document.Close();
                }
            }
            return retVal;
        }
    }
}
