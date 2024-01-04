using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.IO;
using HtmlAgilityPack;
using System.Text.RegularExpressions;

namespace WebGrabberCmd
{
    class Program
    {
        class UrlInfo
        {
            public string url;
            public string name;
            public UrlInfo(string url, string name)
            {
                this.url = url;
                this.name = name;
            }
        }
        static void Main(string[] args)
        {
#if False
            if (GetAndSaveFile("http://www.exam8.com/zige/jiaoshi/fudao/201011/1697125.html"))
                System.Console.WriteLine("OK!");
            else
                System.Console.WriteLine("Failed!");
#endif
            // 2011年教师资格证中学教育学各章节讲义汇总
            string rootURL = "http://www.exam8.com/zige/jiaoshi/fudao/201011/1697157.html";
            HtmlDocument doc = GetHtmlDocForChinese(rootURL);

            StreamWriter file = new StreamWriter("2011年教师资格证中学教育学各章节讲义汇总.html", false, System.Text.Encoding.Default);

            file.Write(doc.DocumentNode.SelectSingleNode("//div[@class='tpdycsacrm']/table").OuterHtml);

            List<UrlInfo> courseList = GetCourseUrlList(doc);
            int i = 1;
            foreach (UrlInfo urlInfo in courseList)
            {
                string subfile = i.ToString("000") + " - " + urlInfo.name + ".html";
                if (File.Exists(subfile))
                {
                    file.Write(File.ReadAllText(subfile, Encoding.Default));
                }
                else
                {
                    string txt = GetOneCourseContent(urlInfo.url);
                    File.WriteAllText(subfile, txt, Encoding.Default);
                    file.Write(txt);
                }
                ++i;
            }
            file.Close();
        }

        static private bool GetAndSaveFile(string url)
        {
            try
            {
                WebClient webclient = new WebClient();
                string filename = url.Substring(url.LastIndexOf("/")+1);
                webclient.DownloadFile(url, filename);
                return true;
            }
            catch (System.Exception)
            {
                return false;
            }
        }

        static private List<UrlInfo> GetCourseUrlList(HtmlDocument doc)
        {
            List<UrlInfo> courseList = new List<UrlInfo>();
            HtmlNodeCollection allLinks = 
                doc.DocumentNode.SelectNodes("//div[@class='tpdycsacrm']/table//td[@class='lianjie']/a");

            if (allLinks == null) return null;

            foreach (HtmlNode link in allLinks)
            {
                string linkurl = link.Attributes["href"].Value;
                string linkname = link.InnerText;
                courseList.Add(new UrlInfo(linkurl, linkname));
                Console.WriteLine(linkurl);
            }

            return courseList;
        }

        static private string GetOneCourseContent(string url)
        {
            HtmlDocument doc = GetHtmlDocForChinese(url);
            List<string> pageLinks = new List<string>();

            HtmlNodeCollection allLinks = doc.DocumentNode.SelectNodes("//div[@class='showpage']/a");
            allLinks.RemoveAt(allLinks.Count - 1);
            allLinks.RemoveAt(allLinks.Count - 1);
            foreach (HtmlNode link in allLinks)
            {
                string linkurl = link.Attributes["href"].Value;
                pageLinks.Add(linkurl);
                Console.WriteLine(linkurl);
            }

            string content = GetOneCourseContentSinglePageCustom(url, true);

            if (allLinks == null) return content;
            foreach (string link in pageLinks)
            {
#if False
                doc = htmlWeb.Load(link);
                content += GetOneCourseContentSinglePage(doc);
#else
                content += GetOneCourseContentSinglePageCustom(link, false);
#endif
            }

            return content;
        }

        static private string GetOneCourseContentSinglePage(HtmlDocument doc, bool first)
        {
            HtmlNode content = doc.DocumentNode.SelectSingleNode("//div[@class='left']/div[6]");
            if (!first)
            {
                content.RemoveChild(content.SelectSingleNode("//table[@class='ContnetPageGuide']"));
            }
            return Regex.Replace(content.InnerHtml, @"<div\sclass=['""]showpage['""][\s\S]*$", string.Empty);
        }

        static private string GetOneCourseContentSinglePageCustom(string url, bool first)
        {
            return GetOneCourseContentSinglePage(GetHtmlDocForChinese(url), first);
        }

        static HtmlDocument GetHtmlDocForChinese(string url)
        {
            HttpWebRequest req;
            req = WebRequest.Create(new Uri(url)) as HttpWebRequest;
            req.Method = "GET";
            WebResponse rs = req.GetResponse();
            Stream rss = rs.GetResponseStream();
            try
            {
                HtmlDocument doc = new HtmlDocument();
                doc.Load(rss);
                return doc;
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message.ToString());
                Console.WriteLine(e.StackTrace);
                return null;
            }
        }
    }
}
