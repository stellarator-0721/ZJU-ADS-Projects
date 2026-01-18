import matplotlib.pyplot as plt
from wordcloud import WordCloud
import numpy as np

# 从文件中提取的词频数据
word_frequencies = {
    'the': 28116, 'and': 26429, 'to': 19431, 'of': 16597, 'you': 13455,
    'my': 12466, 'in': 11072, 'that': 11032, 'is': 9206, 'not': 8518,
    'with': 8002, 'me': 7779, 'for': 7686, 'it': 7669, 'his': 7003,
    'be': 6923, 'this': 6679, 'your': 6622, 'he': 6380, 'but': 6339,
    'have': 5863, 'as': 5804, 'thou': 5638, 'him': 5214, 'so': 5128,
    'will': 4966, 'what': 4483, 'thy': 4157, 'her': 4102, 'all': 3846,
    'by': 3829, 'no': 3817, 'do': 3772, 'shall': 3586, 'if': 3528,
    'are': 3469, 'we': 3319, 'thee': 3266, 'on': 3119, 'our': 3048,
    'now': 2856, 'good': 2784, 'from': 2733, 'lord': 2719, 'at': 2541,
    'come': 2538, 'they': 2533, 'sir': 2504, 'she': 2472, 'or': 2472,
    'which': 2373, 'more': 2341, 'would': 2302, 'then': 2271, 'was': 2226,
    'their': 2219, 'how': 2200, 'well': 2147, 'here': 2138, 'am': 2136,
    'let': 2112, 'when': 2075, 'love': 2067, 'enter': 2033, 'them': 2009,
    'hath': 1963, 'than': 1894, 'like': 1820, 'one': 1816, 'there': 1815,
    'man': 1791, 'an': 1766, "i'll": 1749, 'upon': 1744, 'go': 1715,
    'make': 1674, 'did': 1668, 'may': 1661, 'say': 1649, 'us': 1636,
    'know': 1621, 'should': 1615, 'yet': 1601, 'were': 1583, 'king': 1523,
    'must': 1507, 'why': 1476, 'see': 1458, 'had': 1444, 'such': 1428,
    'out': 1385, "'tis": 1376, 'some': 1359, 'these': 1341, 'give': 1336,
    'where': 1297, 'who': 1270, 'too': 1254, 'can': 1236, 'take': 1190,
    'mine': 1176, 'most': 1165, 'speak': 1148, 'up': 1103, 'doth': 1065,
    'tell': 1064, 'time': 1063, 'much': 1037, 'exeunt': 1035, 'never': 1035,
    'heart': 1020, 'think': 1012, 'nor': 995, 'exit': 946, 'art': 937,
    'death': 893, 'hear': 882, 'men': 874, 'great': 871, 'life': 860,
    'hand': 858, 'away': 857, 'look': 843, 'made': 835, 'father': 828,
    'any': 811, 'sweet': 804, 'true': 796, 'fair': 794, 'thus': 789,
    'very': 784, 'again': 769, 'master': 765, 'own': 763, 'cannot': 759,
    'eyes': 755, 'god': 744, 'before': 741, 'pray': 733, 'ay': 731,
    'day': 722, 'being': 721, 'been': 715, 'night': 684, 'two': 670,
    'lady': 668, 'done': 665, 'fear': 664, 'name': 662, 'blood': 654,
    'old': 651, 'leave': 651, 'other': 650, 'down': 649, 'till': 647,
    'poor': 643, 'whose': 638, 'could': 632, 'though': 632, 'world': 631,
    'therefore': 631, 'into': 629, 'heaven': 629, 'even': 627, 'nothing': 622,
    'comes': 614, 'both': 612, '--': 609, 'honour': 609, 'noble': 607,
    'hast': 603, 'ever': 599, 'still': 595, 'myself': 593, 'son': 592,
    'better': 591, 'against': 582, 'call': 577, 'way': 572, 'grace': 569,
    'those': 568, 'dead': 568, 'nay': 568, 'every': 564, 'many': 562,
    'stand': 556, 'bear': 556, 'duke': 551, 'first': 548, 'peace': 540,
    'live': 527, 'brother': 522, 'queen': 520, 'head': 516, 'find': 511,
    'might': 507, 'thing': 503, 'within': 502, 'word': 498, 'little': 495
}

# 创建词云对象
wordcloud = WordCloud(
    width=1200, 
    height=800,
    background_color='white',
    colormap='viridis',  # 可以使用 'plasma', 'inferno', 'magma' 等
    max_words=200,
    relative_scaling=0.5,
    random_state=42
).generate_from_frequencies(word_frequencies)

# 绘制词云图
plt.figure(figsize=(15, 10))
plt.imshow(wordcloud, interpolation='bilinear')
plt.axis('off')
plt.title("Shakespeare Word Frequency Word Cloud", fontsize=20, pad=20)
plt.tight_layout()

# 保存图片
plt.savefig('shakespeare_wordcloud.png', dpi=300, bbox_inches='tight')
plt.show()

print("词云图已保存为 'shakespeare_wordcloud.png'")