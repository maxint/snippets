import os
import requests
import bs4
from bs4 import BeautifulSoup

URL = "https://tub-rip.github.io/eventvision2021"


def get_slide_url_elem(block):
    urls = block.find_all('a')
    for url in urls:
        if url.text.strip() in ['Slides']:
            return url
    title = block.contents[0]
    if isinstance(title, bs4.element.Tag):
        return title
    elif not isinstance(title, bs4.element.NavigableString):
        return title.find('a')
    return
    

def get_slide_url(block):
    url = get_slide_url_elem(block)
    if url:
        url = url.attrs['href'].strip()
        if url.startswith('/'):
            url = URL + url
        return url
    return


def get_video_url(block):
    urls = block.find_all('a')
    for url in urls:
        if url.text.strip() in ['Video']:
            return url.attrs['href'].strip()
    return
    
    
def get_arxiv_paper_url(url):
    if 'arxiv.org/pdf' in url:
        if url.endswith('.pdf'):
            return url
        else:
            return url + '.pdf'
    return


page = requests.get(URL)
soup = BeautifulSoup(page.content, "html.parser")
hrefs = soup.find(id='accepted-papers').find_next('ul').find_all('a')
blocks = [x.parent.parent for x in hrefs]
blocks += [x.parent.parent.parent for x in soup.find_all('span', string='Video')]
blocks = [x for x in blocks if x.name in ['td', 'li']]
blocks = set(blocks)
for i, block in enumerate(blocks):
    title = block.contents[0].text.split('\n')[0].strip().strip(',').strip('.')
    title = title.replace(':', ' -').replace('?', '')
    assert title
    print(title)
    url = get_slide_url(block)
    if url:
        if 'arxiv.org/' in url:
            path = 'CVPRW21 ' + title + ' [arxiv].pdf'
            url = get_arxiv_paper_url(url)
        else:
            path = 'CVPRW21 ' + title + '.pdf'
        print('  paper:', url)
        if not os.path.exists(path) or os.path.getsize(path) < 50*1024:
            with open(path, 'wb') as f:
                r = requests.get(url, allow_redirects=True, stream=True, timeout=None)
                for c in r.iter_content(2048):
                    f.write(c)
    url = get_video_url(block)
    if url:
        print('  video:', url)
        path = 'CVPRW21 ' + title + ' [video].html'
        open(path, 'wt').write(f'''<head>
<title>{title}</title>
<meta http-equiv="refresh" content="0; URL={url}" />
</head>''')
