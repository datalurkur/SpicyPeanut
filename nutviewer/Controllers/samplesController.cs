using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Entity;
using System.Linq;
using System.Net;
using System.Web;
using System.Web.Mvc;
using nutviewer.Models;

namespace nutviewer
{
    public class samplesController : Controller
    {
        private SpicyPeanutEntities db = new SpicyPeanutEntities();

        // GET: samples
        public ActionResult Index(string timePeriod)
        {
            GraphModel graphModel;
            int idealWidth = 1000;
            int idealHeight = 750;

            if (timePeriod == "all")
            {
                var samples = db.samples.Include(s => s.sampletype).GroupBy(
                    s => new {type = s.sampletype, date = DbFunctions.TruncateTime(s.sampledate).Value},
                    (k, g) => new GraphSample
                    {
                        sampletypeid = k.type.id,
                        sampledate = k.date,
                        sampledata = g.Average(s => s.sampledata)
                    });
                graphModel = new GraphModel(idealWidth, idealHeight, samples);
            }
            else
            {
                DateTime oldestSampleTime = DateTime.UtcNow - TimeSpan.FromDays(4);
                var samples = db.samples.Where(s => s.sampledate > oldestSampleTime).Include(s => s.sampletype).Select(s => new GraphSample
                {
                    sampletypeid = s.sampletype.id,
                    sampledate = s.sampledate,
                    sampledata = s.sampledata
                });
                graphModel = new GraphModel(idealWidth, idealHeight, samples);
            }

            return View(graphModel);
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                db.Dispose();
            }
            base.Dispose(disposing);
        }
    }
}
