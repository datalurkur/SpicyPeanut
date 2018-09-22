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
        public ActionResult Index()
        {
            // Pull 2 weeks of data
            // DEBUG: Actually just pull 2 days
            DateTime oldestSampleTime = DateTime.UtcNow - TimeSpan.FromDays(2);
            var samples = db.samples.Where(s => s.sampledate > oldestSampleTime).Include(s => s.sampletype);

            GraphModel graphModel;
            int idealWidth = 1000;
            int idealHeight = 750;
            graphModel = new GraphModel(idealWidth, idealHeight, samples);
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
