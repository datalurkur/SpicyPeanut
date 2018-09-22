using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Entity;
using System.Linq;
using System.Net;
using System.Web;
using System.Web.Mvc;
using nutviewer.Models;

namespace nutviewer.Controllers
{
    public class HomeController : Controller
    {
        private SpicyPeanutEntities db = new SpicyPeanutEntities();

        public ActionResult Index()
        {
            var samples = db.samples.OrderByDescending(s => s.sampledate).Include(s => s.sampletype);

            HomeDataModel dataModel = new Models.HomeDataModel
            {
                LastTemperature = db.samples.Where(s => s.sampletype.id == 1).OrderByDescending(s => s.sampledate).First().sampledata,
                LastHumidity = db.samples.Where(s => s.sampletype.id == 2).OrderByDescending(s => s.sampledate).First().sampledata,
            };
            return View(dataModel);
        }
    }
}