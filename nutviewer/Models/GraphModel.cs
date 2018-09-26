namespace nutviewer.Models
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    public class SampleGraphPoint
    {
        public int XPosition { get; set; }
        public int YPosition { get; set; }
        public DateTime XData { get; set; }
        public double YData { get; set; }
    }

    public class GridLine
    {
        public int X1 { get; set; }
        public int X2 { get; set; }
        public int Y1 { get; set; }
        public int Y2 { get; set; }
    }

    public class GraphLabel
    {
        public int X { get; set; }
        public int Y { get; set; }
        public string Text { get; set; }
        public string Class { get; set; }
    }

    public class SampleGraph
    {
        public int Width { get; set; }
        public int Height { get; set; }
        public string Stroke { get; set; }
        public IEnumerable<SampleGraphPoint> Points { get; set; }
        public IEnumerable<GridLine> Axes { get; set; }
        public IEnumerable<GridLine> GridLines { get; set; }
        public IEnumerable<GraphLabel> Labels { get; set; }
    }

    public class GraphSample
    {
        public GraphSample() { }
        public DateTime sampledate;
        public double sampledata;
        public int sampletypeid;
    }

    public class GraphModel
    {
        public Dictionary<int, SampleGraph> Graphs { get; set; } = new Dictionary<int, SampleGraph>();

        private int xMin, xRange, xMax;
        private int yMin, yRange, yMax;

        private int MapX(double xRatio)
        {
            return (int)(xRatio * xRange) + xMin;
        }

        private int MapY(double yRatio)
        {
            return (int)(yRatio * yRange) + yMin;
        }

        public GraphModel(int width, int height, IEnumerable<GraphSample> samples)
        {
            int leftPadding = 48;
            int rightPadding = 0;
            int topPadding = 32;
            int bottomPadding = 64;

            this.xMin = leftPadding;
            this.xMax = width - rightPadding;
            this.xRange = this.xMax - this.xMin;
            this.yMin = height - bottomPadding;
            this.yMax = topPadding;
            this.yRange = this.yMax - this.yMin;

            DateTime minDate = samples.OrderBy(s => s.sampledate).First().sampledate;
            DateTime maxDate = samples.OrderByDescending(s => s.sampledate).First().sampledate;
            TimeSpan totalTime = maxDate - minDate;
            int timeIntervalInHours = 3;
            int t = (int)(totalTime.TotalHours / timeIntervalInHours);
            while (t > 8)
            {
                t /= 2;
                timeIntervalInHours *= 2;
            }

            double dateRange = totalTime.TotalMinutes;

            double[] dataTypeMinima =
            {
                0, // Not used
                60, // Temperature
                40, // Humidity
                6, // pH
                1500, // EC
                500, // TDS
            };
            double[] dataTypeMaxima =
            {
                0, // Not used
                85, // Temperature
                70, // Humidity
                8, // pH
                2000, // EC
                1500, // TDS
            };
            double[] dataTypeGridStep =
            {
                0,
                5,
                5,
                0.2,
                100,
                100
            };
            string[] dataTypeTitles =
            {
                "Not Used",
                "Temperature",
                "Humidity",
                "pH",
                "EC",
                "TDS"
            };
            string[] dataTypeColors =
            {
                "#000",
                "#f30",
                "#39f",
                "#90d",
                "#395",
                "#650"
            };
            string[] axisFormat =
            {
                "",
                "F0",
                "F0",
                "F1",
                "F0",
                "F0",
            };

            foreach (int sampleType in samples.Select(s => s.sampletypeid).Distinct())
            {
                double step = dataTypeGridStep[sampleType];
                double range = dataTypeMaxima[sampleType] - dataTypeMinima[sampleType];
                var graphSamples = samples.Where(s => s.sampletypeid == sampleType).Select(s => new SampleGraphPoint
                {
                    XData = s.sampledate,
                    YData = s.sampledata,
                    XPosition = this.MapX((s.sampledate - minDate).TotalMinutes / dateRange),
                    YPosition = this.MapY((s.sampledata - dataTypeMinima[sampleType]) / range)
                });
                List<GridLine> axes = new List<GridLine>
                {
                    new GridLine
                    {
                        X1 = this.MapX(0),
                        X2 = this.MapX(1),
                        Y1 = this.MapY(0),
                        Y2 = this.MapY(0)
                    },
                    new GridLine
                    {
                        X1 = this.MapX(0),
                        X2 = this.MapX(0),
                        Y1 = this.MapY(0),
                        Y2 = this.MapY(1)
                    }
                };
                List<GridLine> gridLines = new List<GridLine>();
                List<GraphLabel> labels = new List<GraphLabel>();
                for (double i = dataTypeMinima[sampleType] + step; i < dataTypeMaxima[sampleType]; i += step)
                {
                    int yPosition = this.MapY((i - dataTypeMinima[sampleType]) / range);
                    gridLines.Add(new GridLine
                    {
                        X1 = this.MapX(0),
                        X2 = this.MapX(1),
                        Y1 = yPosition,
                        Y2 = yPosition
                    });
                    labels.Add(new GraphLabel
                    {
                        X = this.MapX(0) - 2,
                        Y = yPosition + 2,
                        Text = i.ToString(axisFormat[sampleType]),
                        Class = "axisLabel"
                    });
                }

                // Vertical line every 6 hours, anchored to midnight
                TimeZoneInfo tzi = TimeZoneInfo.FindSystemTimeZoneById("Pacific Standard Time");
                DateTime convertedMinDate = TimeZoneInfo.ConvertTimeFromUtc(minDate, tzi);
                DateTime convertedMaxDate = TimeZoneInfo.ConvertTimeFromUtc(maxDate, tzi);
                DateTime current = convertedMinDate.Date;
                while (true)
                {
                    current = current.AddHours(timeIntervalInHours);
                    if (current <= convertedMinDate) { continue; }
                    if (current >= convertedMaxDate) { break; }
                    
                    int x = this.MapX((current - convertedMinDate).TotalMinutes / dateRange);
                    gridLines.Add(new GridLine
                    {
                        X1 = x,
                        X2 = x,
                        Y1 = this.MapY(0),
                        Y2 = this.MapY(1)
                    });
                    labels.Add(new GraphLabel
                    {
                        X = x,
                        Y = this.MapY(0) + 16,
                        Text = current.ToString("t"),
                        Class = "axisLabel"
                    });
                    if (current.Date == current)
                    {
                        labels.Add(new GraphLabel
                        {
                            X = x,
                            Y = this.MapY(0) + 32,
                            Text = current.ToString("d"),
                            Class = "axisLabel"
                        });
                    }
                }

                labels.Add(new GraphLabel
                {
                    X = this.MapX(0),
                    Y = topPadding - 8,
                    Text = dataTypeTitles[sampleType],
                    Class = "title"
                });
                this.Graphs[sampleType] = new SampleGraph
                {
                    Width = width,
                    Height = height,
                    Stroke = dataTypeColors[sampleType],
                    Axes = axes,
                    GridLines = gridLines,
                    Labels = labels,
                    Points = graphSamples.ToList()
                };
            }
        }
    }
}
