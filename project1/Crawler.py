import urllib.request
import urllib.parse
import urllib.error
import os
import re
import html.entities
import collections
import time

def get_page_content(url):
    """获取网页内容"""
    try:
        headers = {'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'}
        req = urllib.request.Request(url, headers=headers)
        
        with urllib.request.urlopen(req, timeout=10) as response:
            content = response.read().decode('utf-8')
            return content
    except Exception as e:
        print(f"错误: 无法获取 {url} - {e}")
        return None

def extract_play_links_from_index(html_content, base_url):
    """从莎士比亚网站主页提取剧本链接"""
    links = []
    
    # 查找所有剧本链接 - 这些通常在表格中
    # 使用更宽泛的模式匹配所有可能的剧本链接
    link_patterns = [
        r'href="([^"]*/([^"/]+)/full\.html)"',
        r'href="([^"]*full\.html)"',
        r'href="(\.\./[^"]*full\.html)"'
    ]
    
    for pattern in link_patterns:
        matches = re.findall(pattern, html_content, re.IGNORECASE)
        for match in matches:
            if isinstance(match, tuple):
                link = match[0]
            else:
                link = match
                
            # 构建完整URL
            if link.startswith('http'):
                full_url = link
            else:
                full_url = urllib.parse.urljoin(base_url, link)
            
            # 从URL提取剧本名称
            play_name = extract_play_name(full_url)
            links.append((play_name, full_url))
    
    # 去重
    unique_links = list(dict.fromkeys(links))
    return unique_links

def extract_play_name(url):
    """从URL提取剧本名称"""
    # 例如: http://shakespeare.mit.edu/hamlet/full.html -> hamlet
    parts = url.split('/')
    for i, part in enumerate(parts):
        if 'shakespeare.mit.edu' in part and i + 1 < len(parts):
            return parts[i + 1].replace('_', ' ').title()
    return "unknown_play"

def extract_play_text(html_content):
    """提取剧本文本"""
    # 移除脚本和样式
    html_content = re.sub(r'<script[^>]*>.*?</script>', '', html_content, flags=re.DOTALL | re.IGNORECASE)
    html_content = re.sub(r'<style[^>]*>.*?</style>', '', html_content, flags=re.DOTALL | re.IGNORECASE)
    
    # 提取<body>内容
    body_match = re.search(r'<body[^>]*>(.*?)</body>', html_content, flags=re.DOTALL | re.IGNORECASE)
    if body_match:
        body_content = body_match.group(1)
    else:
        body_content = html_content
    
    # 尝试找到包含对话的特定区域
    # 莎士比亚网站通常将对话放在特定标签中
    dialogue_blocks = re.findall(r'<a name=\d+></a>(.*?)(?=<br>|<p>|</blockquote>|$)', body_content, re.DOTALL | re.IGNORECASE)
    
    if dialogue_blocks:
        # 如果找到对话块，使用它们
        text_blocks = []
        for block in dialogue_blocks:
            # 移除HTML标签
            clean_block = re.sub(r'<[^>]+>', ' ', block)
            clean_block = re.sub(r'\s+', ' ', clean_block).strip()
            if clean_block and len(clean_block) > 10:
                text_blocks.append(clean_block)
        text = '\n'.join(text_blocks)
    else:
        # 如果没有找到特定对话块，回退到通用方法
        # 移除HTML标签但保留文本
        text = re.sub(r'<[^>]+>', ' ', body_content)
    
    # 解码HTML实体
    text = unescape(text)
    
    # 清理多余空白和特殊字符
    text = re.sub(r'\s+', ' ', text)
    text = re.sub(r'[^\x00-\x7F]+', ' ', text)  # 移除非ASCII字符
    
    return text.strip()

def unescape(text):
    """解码HTML实体（如&amp; -> &）"""
    def replace_entities(match):
        entity = match.group(1)
        if entity in html.entities.name2codepoint:
            return chr(html.entities.name2codepoint[entity])
        elif entity.startswith('#x'):
            return chr(int(entity[2:], 16))
        elif entity.startswith('#'):
            return chr(int(entity[1:]))
        else:
            return match.group(0)
    
    return re.sub(r'&([^;]+);', replace_entities, text)

def save_play_text(text, filename, output_dir):
    """保存剧本文本到文件"""
    os.makedirs(output_dir, exist_ok=True)
    filepath = os.path.join(output_dir, f"{filename}.txt")
    
    try:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(text)
        print(f"✓ 已保存: {filepath}")
        return True
    except Exception as e:
        print(f"✗ 保存失败 {filepath}: {e}")
        return False

def get_known_plays():
    """返回已知的莎士比亚剧本列表作为备用"""
    return [
        ("alls_well", "All's Well That Ends Well", "https://shakespeare.mit.edu/all_well/full.html"),
        ("antony_cleopatra", "Antony and Cleopatra", "https://shakespeare.mit.edu/cleopatra/full.html"),
        ("as_you_like", "As You Like It", "https://shakespeare.mit.edu/as_you_like_it/full.html"),
        ("comedy_errors", "Comedy of Errors", "https://shakespeare.mit.edu/comedy_errors/full.html"),
        ("coriolanus", "Coriolanus", "https://shakespeare.mit.edu/coriolanus/full.html"),
        ("cymbeline", "Cymbeline", "https://shakespeare.mit.edu/cymbeline/full.html"),
        ("hamlet", "Hamlet", "https://shakespeare.mit.edu/hamlet/full.html"),
        ("henry_iv_1", "Henry IV Part 1", "https://shakespeare.mit.edu/1henryiv/full.html"),
        ("henry_iv_2", "Henry IV Part 2", "https://shakespeare.mit.edu/2henryiv/full.html"),
        ("henry_v", "Henry V", "https://shakespeare.mit.edu/henryv/full.html"),
        ("henry_vi_1", "Henry VI Part 1", "https://shakespeare.mit.edu/1henryvi/full.html"),
        ("henry_vi_2", "Henry VI Part 2", "https://shakespeare.mit.edu/2henryvi/full.html"),
        ("henry_vi_3", "Henry VI Part 3", "https://shakespeare.mit.edu/3henryvi/full.html"),
        ("henry_viii", "Henry VIII", "https://shakespeare.mit.edu/henryviii/full.html"),
        ("julius_caesar", "Julius Caesar", "https://shakespeare.mit.edu/julius_caesar/full.html"),
        ("king_lear", "King Lear", "https://shakespeare.mit.edu/lear/full.html"),
        ("macbeth", "Macbeth", "https://shakespeare.mit.edu/macbeth/full.html"),
        ("merchant", "Merchant of Venice", "https://shakespeare.mit.edu/merchant/full.html"),
        ("midsummer", "Midsummer Night's Dream", "https://shakespeare.mit.edu/midsummer/full.html"),
        ("much_ado", "Much Ado About Nothing", "https://shakespeare.mit.edu/much_ado/full.html"),
        ("othello", "Othello", "https://shakespeare.mit.edu/othello/full.html"),
        ("richard_ii", "Richard II", "https://shakespeare.mit.edu/richardii/full.html"),
        ("richard_iii", "Richard III", "https://shakespeare.mit.edu/richardiii/full.html"),
        ("romeo_juliet", "Romeo and Juliet", "https://shakespeare.mit.edu/romeo_juliet/full.html"),
        ("taming_shrew", "Taming of the Shrew", "https://shakespeare.mit.edu/taming_shrew/full.html"),
        ("tempest", "The Tempest", "https://shakespeare.mit.edu/tempest/full.html"),
        ("timon", "Timon of Athens", "https://shakespeare.mit.edu/timon/full.html"),
        ("titus", "Titus Andronicus", "https://shakespeare.mit.edu/titus/full.html"),
        ("twelfth_night", "Twelfth Night", "https://shakespeare.mit.edu/twelfth_night/full.html"),
        ("two_gentlemen", "Two Gentlemen of Verona", "https://shakespeare.mit.edu/two_gentlemen/full.html"),
        ("winters_tale", "Winter's Tale", "https://shakespeare.mit.edu/winters_tale/full.html")
    ]

def main():
    base_url = "https://shakespeare.mit.edu/"
    output_dir = "shakespeare_works"
    
    print("开始爬取莎士比亚全集...")
    print("=" * 50)
    
    # 获取主页面
    main_page_content = get_page_content(base_url)
    play_links = []
    
    if main_page_content:
        print("正在从主页面解析剧本链接...")
        play_links = extract_play_links_from_index(main_page_content, base_url)
        print(f"从主页面找到 {len(play_links)} 个剧本")
    
    # 如果从主页面没找到链接，使用备用列表
    if not play_links:
        print("无法从主页面解析链接，使用备用剧本列表...")
        known_plays = get_known_plays()
        play_links = [(name, url) for _, name, url in known_plays]
    
    # 去重
    seen_urls = set()
    unique_play_links = []
    for name, url in play_links:
        if url not in seen_urls:
            seen_urls.add(url)
            unique_play_links.append((name, url))
    
    total_plays = len(unique_play_links)
    successful_downloads = 0
    
    print(f"\n开始下载 {total_plays} 个剧本...")
    print("=" * 50)
    
    for i, (play_name, play_url) in enumerate(unique_play_links, 1):
        print(f"[{i}/{total_plays}] 正在下载: {play_name}")
        print(f"     URL: {play_url}")
        
        play_content = get_page_content(play_url)
        if play_content:
            play_text = extract_play_text(play_content)
            
            if play_text and len(play_text) > 1000:  # 确保有足够的内容
                # 创建安全的文件名
                safe_filename = re.sub(r'[^\w\s-]', '', play_name).strip().replace(' ', '_')
                if save_play_text(play_text, safe_filename, output_dir):
                    successful_downloads += 1
            else:
                print(f"  ⚠  {play_name} 内容过少，可能格式不匹配")
        else:
            print(f"  ✗ 无法下载 {play_name}")
        
        # 延迟以避免对服务器造成压力
        time.sleep(1)
    
    print("=" * 50)
    print(f"下载完成! 成功下载 {successful_downloads}/{total_plays} 个剧本")
    print(f"文件保存在: {os.path.abspath(output_dir)}")

if __name__ == "__main__":
    main()