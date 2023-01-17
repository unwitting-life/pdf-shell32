#pragma warning disable IDE0003

using System.Windows;
using System.Windows.Input;

namespace iTextSharpWrapper {
    public partial class ProgressWindow : Window {
        public ProgressWindow() {
            InitializeComponent();
        }

        private void Window_MouseMove(object sender, MouseEventArgs e) {
            if (e.LeftButton == MouseButtonState.Pressed) {
                this.DragMove();
            }
        }
    }
}
