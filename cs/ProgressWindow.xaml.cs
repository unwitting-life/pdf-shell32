#pragma warning disable IDE0003, IDE1006

using ControlzEx.Theming;

using iTextSharp.text;
using iTextSharp.text.pdf;

using MahApps.Metro.Controls;

using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Forms;

using WebPWrapper;

using Image = iTextSharp.text.Image;

namespace iTextSharpWrapper {
    public partial class ProgressWindow : MetroWindow {
        public bool IsBreakCreating { get; set; } = false;
        public args args { get; set; }

        public ProgressWindow() {
            InitializeComponent();
            // var themes = new string[] {
            //    "Dark.Red", "Dark.Green", "Dark.Blue", "Dark.Purple", "Dark.Orange", "Dark.Lime", "Dark.Emerald", "Dark.Teal",
            //    "Dark.Cyan", "Dark.Cobalt", "Dark.Indigo", "Dark.Violet", "Dark.Pink", "Dark.Magenta", "Dark.Crimson", "Dark.Amber",
            //    "Dark.Yellow", "Dark.Brown", "Dark.Olive", "Dark.Steel", "Dark.Mauve", "Dark.Taupe", "Dark.Sienna",
            // };
            // ThemeManager.Current.ChangeTheme(this, "Dark.Orange");
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e) {
            this.IsBreakCreating = true;
        }

        private void MetroWindow_Loaded(object sender, RoutedEventArgs e) {
            if (this.args != null && this.args.imageFiles.Length > 0 && !string.IsNullOrEmpty(this.args.pdfFilePath)) {
                this.Title = $"Creating {Path.GetFileName(this.args.pdfFilePath)}...";
                var task = new Task(() => {
                    if (this.args != null && this.args.imageFiles.Length > 0 && !string.IsNullOrEmpty(this.args.pdfFilePath)) {
                        var pdfFilePath = this.args.pdfFilePath;
                        if (Path.GetFullPath(pdfFilePath) != pdfFilePath) {
                            pdfFilePath = Path.Combine(Path.GetDirectoryName(this.args.imageFiles[0]), Path.GetFileName(pdfFilePath));
                        }
                        Directory.CreateDirectory(Path.GetDirectoryName(pdfFilePath));
                        var document = new Document(PageSize.A4);
                        var paperSize = string.Empty;
                        if (this.args.paperSize == PaperSize.A3) {
                            document = new Document(PageSize.A3);
                            paperSize = PaperSize.A3;
                        } else if (this.args.paperSize == PaperSize.A4) {
                            document = new Document(PageSize.A4);
                            paperSize = PaperSize.A4;
                        } else if (this.args.paperSize == PaperSize.A5) {
                            document = new Document(PageSize.A5);
                            paperSize = PaperSize.A5;
                        } else if (this.args.paperSize == PaperSize.B5) {
                            document = new Document(PageSize.B5);
                            paperSize = PaperSize.B5;
                        }
                        if (!string.IsNullOrEmpty(paperSize)) {
                            this.Dispatcher.Invoke(() => {
                                this.Title = $"Creating {Path.GetFileName(this.args.pdfFilePath)} ({paperSize})...";
                            });
                        }
                        using (var pdfFileStream = new FileStream(pdfFilePath, FileMode.Create, FileAccess.Write, FileShare.None)) {
                            PdfWriter.GetInstance(document, pdfFileStream);
                            document.Open();
                            var index = 0;
                            foreach (string imageFile in this.args.imageFiles) {
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
                                        this.Dispatcher.Invoke(() => {
                                            this.progress.Value = 100.0 * index / this.args.imageFiles.Length;
                                            var fileName = string.Empty;
                                            var pathes = imageFile.Split(Path.DirectorySeparatorChar);
                                            for (var i = pathes.Length - 1; i >= 0; i--) {
                                                if (string.IsNullOrEmpty(fileName) || pathes[i].Length + fileName.Length <= 70) {
                                                    fileName = Path.Combine(pathes[i], fileName);
                                                } else {
                                                    fileName = Path.Combine("...", fileName);
                                                    break;
                                                }
                                            }
                                            if (imageFile.Length >= 2 && imageFile.Substring(0, 2) == @"\\") {
                                                this.file.Text = fileName;
                                            } else {
                                                this.file.Text = $"{imageFile[0]}:{Path.DirectorySeparatorChar}{fileName}";
                                            }
                                        });
                                        if (this.IsBreakCreating) {
                                            break;
                                        }
                                    }
                                    index++;
                                } catch (Exception ex) {
                                    Debug.WriteLine(ex);
                                }
                            }
                            document.Close();
                            this.Dispatcher.Invoke(() => { this.progress.Value = this.progress.Maximum; });
                            Thread.Sleep(500);
                            this.Dispatcher.Invoke(() => { this.Close(); });
                        }
                    }
                });
                task.Start();
            }
        }

        private void MetroWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e) {
            this.IsBreakCreating = true;
        }
    }
}
