using CommunityToolkit.Mvvm.ComponentModel;
using Microsoft.UI;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace SwapChainPanel.Model
{
    /// <summary>
    /// A simple data model that tracks the drawing parameters for an inking surface.
    /// </summary>
    internal partial class DrawingAttributes : ObservableObject
    {
        public DrawingAttributes() : this(Color.Black, new Size(4, 4), false)
        {
        }

        public DrawingAttributes(Color brushColor, Size brushSize, bool brushFitsToCurve, [Optional] IEnumerable<Color> activePaletteColor)
        {
            BrushColor = brushColor;
            BrushSize = brushSize;
            BrushFitsToCurve = brushFitsToCurve;

            ActivePaletteColor = [.. activePaletteColor];
            BrushSizes = new BrushSizesCollection();
        }

        /// <summary>
        /// Whether the brush is in erase mode.
        /// </summary>
        [ObservableProperty]
        public partial bool BrushIsEraser
        {
            get;
            set;
        }

        /// <summary>
        /// Color of the current drawing brush.
        /// </summary>
        [ObservableProperty]
        public partial Color BrushColor
        {
            get;
            set;
        } = Color.Black;

        /// <summary>
        /// Whether the current drawing brush smooths strokes.
        /// </summary>
        [ObservableProperty]
        public partial bool BrushFitsToCurve
        {
            get; set;
        }

        /// <summary>
        /// Size of the current drawing brush.
        /// </summary>
        [ObservableProperty]
        public partial Size BrushSize
        {
            get; set;
        } = new Size(4, 4);

        /// <summary>
        /// The list of Color that are active on a palette control.
        /// </summary>
        [ObservableProperty]
        public partial PaletteColorCollection ActivePaletteColor
        {
            get; set;
        }

        private static AllColorCollection _allColor = new AllColorCollection();
        /// <summary>
        /// All Color available in a palette control.
        /// </summary>
        public static AllColorCollection AllColor
        {
            get { return _allColor; }
        }

        /// <summary>
        /// All supported sizes for a drawing brush.
        /// </summary>
        public BrushSizesCollection BrushSizes
        {
            get;
            set;
        }
    }

    /// <summary>
    /// A collection of supported drawing brush sizes.
    /// </summary>
    public class BrushSizesCollection : IReadOnlyCollection<Size>
    {
        private static readonly Size[] Sizes = { new Size(2, 2), new Size(4, 4), new Size(10, 10), new Size(20, 20), new Size(40, 40) };

        public BrushSizesCollection()
        {
        }

        public int Count
        {
            get { return Sizes.Length; }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return Sizes.GetEnumerator();
        }

        IEnumerator<Size> IEnumerable<Size>.GetEnumerator()
        {
            return Sizes.Cast<Size>().GetEnumerator();
        }
    }

    /// <summary>
    /// A collection of all available Color to be used in a palette control.
    /// </summary>
    public class AllColorCollection : IReadOnlyCollection<Color>
    {
        private static readonly Color[] AllColor = {
            Color.White, Color.Black, Color.DarkGray, Color.LightGray,
            Color.DarkRed, Color.Red, Color.Orange, Color.Salmon,
            Color.Yellow, Color.Bisque, Color.Goldenrod, Color.YellowGreen,
            Color.ForestGreen, Color.Green, Color.LawnGreen, Color.SeaGreen,
            Color.Navy, Color.Blue, Color.CornflowerBlue, Color.SkyBlue,
            Color.Indigo, Color.Purple, Color.Plum, Color.MediumPurple
            };

        public AllColorCollection()
        {

        }

        public int Count
        {
            get { return AllColor.Length; }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return AllColor.GetEnumerator();
        }

        IEnumerator<Color> IEnumerable<Color>.GetEnumerator()
        {
            return AllColor.Cast<Color>().GetEnumerator();
        }
    }

    /// <summary>
    /// A collection of active Color being used in a palette control.
    /// </summary>
    public class PaletteColorCollection : IList<Color>, IList, INotifyCollectionChanged
    {
        public event NotifyCollectionChangedEventHandler CollectionChanged;

        private const int MaxCount = 4;
        private static readonly Color[] DefaultColor = { Color.White, Color.Black, Color.Red, Color.Blue };
        private Queue<Color> _Color;

        public PaletteColorCollection([Optional] IEnumerable<Color> Color)
        {
            _Color = new Queue<Color>(Color != null ? Color : DefaultColor);
            RemoveExtraneousItems();
        }

        private void RemoveExtraneousItems()
        {
            while (_Color.Count > MaxCount)
            {
                _Color.Dequeue();
            }
        }

        private void RaiseCollectionReset()
        {
            var eventHandler = this.CollectionChanged;
            if (CollectionChanged != null)
            {
                CollectionChanged(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
            }
        }

        #region IList<Color>
        public void Add(Color item)
        {
            if (!_Color.Contains(item))
            {
                _Color.Enqueue(item);
                RemoveExtraneousItems();
                RaiseCollectionReset();
            }
        }

        public void Clear()
        {
            _Color = new Queue<Color>(DefaultColor);
            RaiseCollectionReset();
        }

        public bool Contains(Color item)
        {
            return _Color.Contains(item);
        }

        public void CopyTo(Color[] array, int arrayIndex)
        {
            _Color.CopyTo(array, arrayIndex);
        }

        public int Count
        {
            get { return _Color.Count; }
        }

        public bool IsReadOnly
        {
            get { return false; }
        }

        public bool Remove(Color item)
        {
            return false;
        }

        public IEnumerator<Color> GetEnumerator()
        {
            return _Color.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return _Color.GetEnumerator();
        }

        public int IndexOf(Color item)
        {
            var enumerator = _Color.GetEnumerator();
            int i = 0;
            while (enumerator.MoveNext())
            {
                if (enumerator.Current == item)
                {
                    return i;
                }
                i++;
            }

            return -1;
        }

        #region Not Implemented

        public void Insert(int index, Color item)
        {
            throw new NotImplementedException();
        }

        public void RemoveAt(int index)
        {
            throw new NotImplementedException();
        }

        public Color this[int index]
        {
            get
            {
                throw new NotImplementedException();
            }
            set
            {
                throw new NotImplementedException();
            }
        }

        #endregion

        #endregion

        #region IList
        int IList.Add(object value)
        {
            if (value is Color)
            {
                this.Add((Color)value);
                return this.Count - 1;
            }

            return -1;
        }

        void IList.Clear()
        {
            this.Clear();
        }

        bool IList.Contains(object value)
        {
            return value is Color ? this.Contains((Color)value) : false;
        }

        bool IList.IsFixedSize
        {
            get { return false; }
        }

        bool IList.IsReadOnly
        {
            get { return false; }
        }

        int ICollection.Count
        {
            get { return this.Count; }
        }

        object IList.this[int index]
        {
            get
            {
                return _Color.ElementAt<Color>(index);
            }
            set
            {
                throw new NotImplementedException();
            }
        }

        int IList.IndexOf(object value)
        {
            return (value is Color) ? IndexOf((Color)value) : -1;
        }

        #region Not implemented

        void IList.Insert(int index, object value)
        {
            throw new NotImplementedException();
        }

        void IList.Remove(object value)
        {
            throw new NotImplementedException();
        }

        void IList.RemoveAt(int index)
        {
            throw new NotImplementedException();
        }

        void ICollection.CopyTo(Array array, int index)
        {
            throw new NotImplementedException();
        }


        bool ICollection.IsSynchronized
        {
            get { throw new NotImplementedException(); }
        }

        object ICollection.SyncRoot
        {
            get { throw new NotImplementedException(); }
        }
        #endregion

        #endregion

    }
}
