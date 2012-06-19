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
            public int imagenum;
            public UrlInfo(string url, string name)
            {
                this.url = url;
                this.name = name;
                this.imagenum = 0;
            }
        }
        static void Main(string[] args)
        {
            //DownloadOneBook("人教版全日制普高必修生物第一册", "http://www.gaokao.com/zyk/dzkb/swkb/rs1/");
            //DownloadOneBook("人教版全日制普高必修生物第二册", "http://www.gaokao.com/zyk/dzkb/swkb/rs2/");
            //DownloadOneBook("人教版全日制普高选修生物全一册", "http://www.gaokao.com/zyk/dzkb/swkb/rsq/");
            //DownloadOneBook("人教版普高课标实验教科书生物必修1", "http://www.gaokao.com/zyk/dzkb/swkb/rksb1/");
            //DownloadOneBook("人教版普高课标实验教科书生物必修2", "http://www.gaokao.com/zyk/dzkb/swkb/rksb2/");
            //DownloadOneBook("人教版普高课标实验教科书生物必修3", "http://www.gaokao.com/zyk/dzkb/swkb/rksb3/");

            //DownloadOneBook("人教版普高课标实验教科书生物选修1", "http://www.gaokao.com/zyk/dzkb/swkb/rksx1/");
            DownloadOneBook("人教版普高课标实验教科书生物选修2", "http://www.gaokao.com/zyk/dzkb/swkb/rksx2/");
            DownloadOneBook("人教版普高课标实验教科书生物选修3", "http://www.gaokao.com/zyk/dzkb/swkb/rksx3/");
        }

        static private void DownloadOneBook(string pageName, string rootUrl)
        {
            Console.WriteLine(new string('=', 79));
            Console.WriteLine(string.Format("Downloading book {0} at {1}", pageName, rootUrl));

            // Collect image URLs in each page
            //string pageName = "人教版全日制普高必修生物第一册";
            Directory.CreateDirectory(pageName);
            string imageListPath = pageName + "/list.txt";
            List<string> imageUrlList = new List<string>();
            //Console.WriteLine(string.Format("= Test - current dir is {0}", Directory.GetCurrentDirectory()));
            if (File.Exists(imageListPath))
            {
                StreamReader file = new StreamReader(imageListPath, System.Text.Encoding.Default);
                string line;
                while ((line = file.ReadLine()) != null)
                {
                    line = line.Trim();
                    if (!string.IsNullOrEmpty(line))
                        imageUrlList.Add(line);
                }
                file.Close();
                Console.WriteLine(string.Format("Readed image list (count {1}) from {0}", imageListPath, imageUrlList.Count));
            }
            else
            {
                // Get page links
                HtmlDocument doc = GetHtmlDocForChinese(rootUrl);
                List<UrlInfo> urlList = GetPageUrlListFromRootPage(doc);

                // Check the multiple pages
                HtmlNodeCollection linkNodes = doc.DocumentNode.SelectNodes("//div[@class='list_pages']//a[@href]");
                if (linkNodes != null)
                {
                    linkNodes.RemoveAt(linkNodes.Count - 1);
                    foreach (HtmlNode node in linkNodes)
                    {
                        string linkurl = node.Attributes["href"].Value;
                        doc = GetHtmlDocForChinese(linkurl);
                        List<UrlInfo> infoList = GetPageUrlListFromRootPage(doc);
                        urlList = urlList.Union(infoList).ToList();
                    }
                }
                urlList.Reverse();
                Console.WriteLine(string.Format("Got {0} book pages", urlList.Count));

                foreach (UrlInfo info in urlList)
                {
                    doc = GetHtmlDocForChinese(info.url);
                    List<string> allLinks = GetImageUrlListFromPage(doc);

                    // Check the multiple pages
                    linkNodes = doc.DocumentNode.SelectNodes("//div[@class='pages']//a[@href]");
                    if (linkNodes != null)
                    {
                        linkNodes.RemoveAt(linkNodes.Count - 1);
                        foreach (HtmlNode node in linkNodes)
                        {
                            string linkurl = node.Attributes["href"].Value;
                            doc = GetHtmlDocForChinese(linkurl);
                            List<string> links = GetImageUrlListFromPage(doc);
                            if (links.Count != 0)
                                allLinks = allLinks.Union(links).ToList();
                        }
                    }
                    info.imagenum = allLinks.Count;
                    if (allLinks.Count != 0)
                        imageUrlList = imageUrlList.Union(allLinks).ToList();
                }

                // Save the image list
                StreamWriter file = new StreamWriter(imageListPath, false, System.Text.Encoding.Default);
                foreach (string url in imageUrlList)
                {
                    file.WriteLine(url);
                }
                file.Close();
                Console.WriteLine(string.Format("Saved image list to {0}", imageUrlList));

                // Save book content info
                string bookContentPath = string.Format("{0}/{0}.txt", pageName);
                file = new StreamWriter(bookContentPath, false, System.Text.Encoding.Default);
                int pageidx = 1;
                int imgidx = 1;
                foreach (UrlInfo info in urlList)
                {
                    file.WriteLine(string.Format("{4}: {0} ({1} | {2} - {3})", info.name,
                        info.imagenum, imgidx, imgidx + info.imagenum - 1, ++pageidx));
                    imgidx += info.imagenum;
                }
                file.Close();
                Console.WriteLine(string.Format("Saved book content to {0}", bookContentPath));
            }

            // Download images
            WebClient client = new WebClient();
            int idx = 0;
            int idx0 = 0;
            foreach (string url in imageUrlList)
            {
                string imgPath = string.Format("{1}/{0:000}_{2}.jpg", ++idx, pageName,
                    Regex.Match(url, @"\d*$").Value);
                if (File.Exists(imgPath))
                {
                    Console.WriteLine(string.Format("Image \"{0}\" has been downloaded, skip it.", imgPath));
                }
                else
                {
                    Console.WriteLine(string.Format("Downloading {0} to {1}...[{2}/{3}]",
                        url, imgPath, ++idx0, imageUrlList.Count));
                    client.DownloadFile(url, imgPath);
                }
            }
        }

        static private List<UrlInfo> GetPageUrlListFromRootPage(HtmlDocument doc)
        {
            List<UrlInfo> infoList = new List<UrlInfo>();
            HtmlNodeCollection linkNodes = doc.DocumentNode.SelectNodes("id('content')/li/div[1]/a");
            foreach (HtmlNode node in linkNodes)
            {
                string linkurl = node.Attributes["href"].Value;
                string linkname = node.InnerText;
                infoList.Add(new UrlInfo(linkurl, linkname));
            }
            return infoList;
        }

        static private List<string> GetImageUrlListFromPage(HtmlDocument doc)
        {
            List<string> links = new List<string>();
            HtmlNodeCollection linkNodes = doc.DocumentNode.SelectNodes("//div[@class='content_txt']/p//img");
            if (linkNodes != null)
            {
                foreach (HtmlNode node in linkNodes)
                {
                    string linkurl = node.Attributes["src"].Value;
                    links.Add(linkurl);
                }
            }
            return links;
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
