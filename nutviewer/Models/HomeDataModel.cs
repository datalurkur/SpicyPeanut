namespace nutviewer.Models
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    public class HomeDataModel
    {
        public double LastTemperature { get; set; }
        public double LastHumidity { get; set; }
        public double LastPH { get; set; }
        public double LastEC { get; set; }
        public double LastTDS { get; set; }
    }
}
