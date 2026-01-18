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

def unescape(text):
    """解码HTML实体"""
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

def extract_play_text_improved(html_content):
    """改进的剧本文本提取"""
    # 移除脚本和样式
    html_content = re.sub(r'<script[^>]*>.*?</script>', '', html_content, flags=re.DOTALL | re.IGNORECASE)
    html_content = re.sub(r'<style[^>]*>.*?</style>', '', html_content, flags=re.DOTALL | re.IGNORECASE)
    
    # 提取<body>内容
    body_match = re.search(r'<body[^>]*>(.*?)</body>', html_content, flags=re.DOTALL | re.IGNORECASE)
    if body_match:
        body_content = body_match.group(1)
    else:
        body_content = html_content
    
    # 尝试多种提取策略
    
    # 策略1: 提取对话块（莎士比亚网站常用格式）
    dialogue_patterns = [
        r'<A NAME=(\d+)></A>(.*?)(?=<A NAME=|</BLOCKQUOTE>|<br clear=all>|$)',
        r'<a name="speech\d+"><b>.*?</b></a>\s*<br>\s*(.*?)\s*<br>',
        r'<h3>ACT.*?</h3>(.*?)(?=<h3>|$)'
    ]
    
    text_parts = []
    
    for pattern in dialogue_patterns:
        matches = re.findall(pattern, body_content, re.DOTALL | re.IGNORECASE)
        for match in matches:
            if isinstance(match, tuple):
                text_part = match[1]
            else:
                text_part = match
                
            # 清理HTML标签
            clean_text = re.sub(r'<[^>]+>', ' ', text_part)
            clean_text = re.sub(r'\s+', ' ', clean_text).strip()
            if clean_text and len(clean_text) > 10:
                text_parts.append(clean_text)
    
    # 策略2: 如果上面没找到足够内容，使用通用方法
    if len(text_parts) < 10:
        # 移除所有HTML标签
        clean_text = re.sub(r'<[^>]+>', ' ', body_content)
        clean_text = re.sub(r'\s+', ' ', clean_text)
        clean_text = unescape(clean_text)
        return clean_text.strip()
    else:
        # 合并找到的文本部分
        full_text = '\n'.join(text_parts)
        full_text = unescape(full_text)
        return full_text

def save_play_text(text, filename, output_dir):
    """保存剧本文本到文件"""
    os.makedirs(output_dir, exist_ok=True)
    filepath = os.path.join(output_dir, f"{filename}.txt")
    
    try:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(text)
        print(f"✓ 已保存: {filepath} (长度: {len(text)} 字符)")
        return True
    except Exception as e:
        print(f"✗ 保存失败 {filepath}: {e}")
        return False

def download_specific_plays():
    """专门下载那两个有问题的剧本"""
    plays_to_download = [
        {
            "id": "as_you_like_it",
            "name": "As You Like It",
            "urls": [
                "https://shakespeare.mit.edu/as_you_like_it/full.html",
                "https://shakespeare.mit.edu/asyoulikeit/full.html",
                "https://shakespeare.mit.edu/as_you_like_it/index.html",
                "https://shakespeare.mit.edu/asyoulikeit/index.html"
            ]
        },
        {
            "id": "alls_well",
            "name": "All's Well That Ends Well", 
            "urls": [
                "https://shakespeare.mit.edu/all_well/full.html",
                "https://shakespeare.mit.edu/allswell/full.html",
                "https://shakespeare.mit.edu/all_well/index.html",
                "https://shakespeare.mit.edu/allswell/index.html"
            ]
        }
    ]
    
    output_dir = "shakespeare_fixed"
    successful_downloads = 0
    
    print("开始下载特定剧本...")
    print("=" * 50)
    
    for play in plays_to_download:
        print(f"\n尝试下载: {play['name']}")
        downloaded = False
        
        for url in play['urls']:
            print(f"  尝试 URL: {url}")
            content = get_page_content(url)
            
            if content:
                text = extract_play_text_improved(content)
                
                if text and len(text) > 1000:
                    if save_play_text(text, play['id'], output_dir):
                        successful_downloads += 1
                        downloaded = True
                        print(f"  ✓ 成功下载 {play['name']}")
                        break
                    else:
                        print(f"  ⚠ 找到内容但保存失败")
                else:
                    print(f"  ⚠ 内容过少 ({len(text) if text else 0} 字符)")
            else:
                print(f"  ✗ 无法访问")
            
            time.sleep(1)  # 延迟
        
        if not downloaded:
            print(f"  ✗ 所有URL都失败: {play['name']}")
    
    print("=" * 50)
    print(f"下载完成! 成功下载 {successful_downloads}/{len(plays_to_download)} 个剧本")
    print(f"文件保存在: {os.path.abspath(output_dir)}")

def create_sample_files_for_testing():
    """创建示例文件用于测试搜索引擎（如果下载失败）"""
    output_dir = "shakespeare_sample"
    os.makedirs(output_dir, exist_ok=True)
    
    sample_plays = [
        ("hamlet", "Hamlet", "To be, or not to be: that is the question..."),
        ("macbeth", "Macbeth", "Is this a dagger which I see before me..."),
        ("romeo_juliet", "Romeo and Juliet", "What's in a name? That which we call a rose..."),
        ("as_you_like_it", "As You Like It", "All the world's a stage, And all the men and women merely players..."),
        ("alls_well", "All's Well That Ends Well", "The web of our life is of a mingled yarn, good and ill together...")
    ]
    
    for play_id, play_name, sample_text in sample_plays:
        filepath = os.path.join(output_dir, f"{play_id}.txt")
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(f"TITLE: {play_name}\n")
            f.write("=" * 50 + "\n")
            f.write(sample_text + "\n")
            # 添加更多示例文本
            for i in range(10):
                f.write(f"Sample line {i} for testing search functionality.\n")
    
    print(f"创建了 {len(sample_plays)} 个示例文件在: {os.path.abspath(output_dir)}")

if __name__ == "__main__":
    # 首先尝试下载特定的剧本
    download_specific_plays()
    
    # 如果下载失败，询问是否创建示例文件
    response = input("\n是否创建示例文件用于测试搜索引擎？(y/n): ")
    if response.lower() == 'y':
        create_sample_files_for_testing()