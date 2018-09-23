using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Entity;
using System.IO;
using System.Linq;
using System.Net;
using System.Web;
using System.Web.Mvc;
using nutviewer.Models;

namespace nutviewer.Controllers
{
    public class UploadController : Controller
    {
        [HttpPost]
        public ActionResult Index()
        {
            ContentResult result = new ContentResult();
            result.ContentType = "text/plain";
            result.Content = "No file uploaded";

            try
            {
                foreach (string fileKey in this.Request.Files)
                {
                    HttpPostedFileBase fileData = this.Request.Files[fileKey];
                    string targetPath = HttpContext.Server.MapPath("~/Content/Images/last_capture.png");
                    fileData.SaveAs(targetPath);
                    result.Content = "Upload successful";
                    break;
                }
            }
            catch (Exception e)
            {
                result.Content = "Failed to upload: " + e.Message;
            }

            return result;
        }
    }
}