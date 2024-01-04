import re
import os
import xlwt
from bs4 import BeautifulSoup

xls = xlwt.Workbook(encoding= 'ascii')
worksheet = xls.add_sheet("Sheet1")
titles = '大学 层次 地点 类型 民办/公办 属于 专业排名 专业综合指数 学科评估 热度'.split()
for i, x in enumerate(titles):
    worksheet.write(0, i, x)
max_col_widths = [0] * len(titles)

input_path = '本科-理工-计算机科学与技术-290.html'
input_path = '本科-理工-软件工程-235.html'
input_path = '本科-理工-电子信息工程-261.html'
input_path = '本科-综合理工-计算机科学与技术-651.html'
input_path = '本科-综合理工-电子信息工程-516.html'
content = open(input_path, encoding='utf-8').read()
soup = BeautifulSoup(content, 'html.parser')
i = 0
for x in soup.contents[0].contents:
    if x.name != 'div':
        continue
    divs = [x for x in x.contents if x.name == 'div']
    college = divs[0].find(class_='college-name').find('a').text.strip()
    levels = [x.text.strip() for x in divs[0].find(class_='mt5').find_all('span')]
    extra = [x.text.strip() for x in divs[0].find(class_='mt5 f14 fcolor666').find_all('span')]
    location = extra[0].replace(' ', '').replace('/', '').replace('\n', '/')
    category = extra[1].split()[0]
    ownership = extra[2].split()[0]
    belong_to = extra[3].split()[0]
    order_ex = [x.text.strip() for x in divs[1].find(class_='mt5').find_all('span')]
    order = re.search(r'[.0-9]+', order_ex[0])[0]
    index = re.search(r'[.0-9]+', order_ex[1])[0] if len(order_ex) > 1 else ''
    rate = divs[2].find(class_='mt5').text.strip()
    hot = divs[3].find(class_='mt5').text.strip()
    assert len(divs) == 4

    values = [
        college,
        ','.join(levels),
        location,
        category,
        ownership,
        belong_to,
        order,
        index,
        rate,
        hot,
    ]
    i += 1
    for j, y in enumerate(values):
        worksheet.write(i, j, y)
        max_col_widths[j] = max(max_col_widths[j], len(y))
pass

for i, x in enumerate(max_col_widths):
    worksheet.col(i).width = (x + 4) * 367
out_path = os.path.splitext(input_path)[0] + '.xls'
xls.save(out_path)