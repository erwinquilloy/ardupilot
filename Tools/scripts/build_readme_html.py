#!/usr/bin/env python3
'''
Render README_CUSTOM.md into one self-contained HTML page: an outline
rail on the left (a fold-out list on phones) and the README on the right.

- Heading anchors match GitHub's, so every #link in the README works.
- Images under docs/images/ are inlined, so the page is a single file.
- Links to repository files point at the given branch on GitHub.

Needs the "markdown" package (pip install markdown).

usage: build_readme_html.py OUTPUT.html [--branch BRANCH]

AP_FLAKE8_CLEAN
'''

import argparse
import base64
import os
import re
import subprocess

import markdown
from markdown.extensions.toc import TocExtension

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
GITHUB = 'https://github.com/erwinquilloy/ardupilot'


def git(*args):
    return subprocess.check_output(['git', '-C', REPO] + list(args), text=True).strip()


def define(path, name):
    '''value of a #define in a repo header'''
    text = open(os.path.join(REPO, path), encoding='utf-8').read()
    return re.search(r'#define\s+%s\s+"?([^"\n]+)"?' % name, text).group(1)


def gh_slug(value, separator):
    '''GitHub heading anchor: lower case, punctuation and emoji dropped, spaces to -'''
    s = re.sub(r'[^\w\- ]', '', value.strip().lower())
    return s.replace(' ', '-')


def fix_dup(anchor):
    '''the toc extension numbers repeats as x_1, GitHub as x-1'''
    return re.sub(r'_(\d+)$', lambda m: '-' + m.group(1), anchor)


def data_uri(path):
    mime = 'image/png' if path.lower().endswith('.png') else 'image/jpeg'
    data = base64.b64encode(open(os.path.join(REPO, path), 'rb').read()).decode()
    return 'data:%s;base64,%s' % (mime, data)


def inline_image(m):
    return '![%s](%s)' % (m.group(1), data_uri(m.group(2)))


def build(out, branch):
    src = open(os.path.join(REPO, 'README_CUSTOM.md'), encoding='utf-8').read()
    src = re.sub(r'!\[([^\]]*)\]\((docs/images/[^)]+)\)', inline_image, src)
    # raw <img>/<source> tags (the centred logo); class="logo" drops the photo border
    src = re.sub(r'<img src="(docs/images/[^"]+)"',
                 lambda m: '<img class="logo" src="%s"' % data_uri(m.group(1)), src)
    src = re.sub(r'srcset="(docs/images/[^"]+)"',
                 lambda m: 'srcset="%s"' % data_uri(m.group(1)), src)
    src = re.sub(r'\]\(((?:docs|libraries|Tools)/[^)#]*)\)',
                 lambda m: '](%s/blob/%s/%s)' % (GITHUB, branch, m.group(1)), src)

    md = markdown.Markdown(extensions=['tables', 'fenced_code', 'sane_lists',
                                       TocExtension(slugify=gh_slug, toc_depth='1-2')])
    body = md.convert(src)
    body = re.sub(r'(id="[^"]*?)_(\d+)"', lambda m: m.group(1) + '-' + m.group(2) + '"', body)
    body = re.sub(r'<a href="(https?://[^"]+)"', r'<a href="\1" target="_blank" rel="noopener"', body)
    body = body.replace('<table>', '<div class="tw"><table>').replace('</table>', '</table></div>')

    groups = []
    for t in md.toc_tokens:
        kids = ''.join('<li><a href="#%s">%s</a></li>' % (fix_dup(c['id']), c['name']) for c in t['children'])
        groups.append('<li class="g"><a href="#%s">%s</a>%s</li>' %
                      (fix_dup(t['id']), t['name'], '<ul>%s</ul>' % kids if kids else ''))
    toc = '<ul>%s</ul>' % ''.join(groups)

    fork = define('libraries/AP_Common/AP_FWVersionDefine.h', 'FORK_VERSION')
    upstream = define('ArduPlane/version.h', 'THISFIRMWARE').replace('ArduPlane V', '')
    commit = git('rev-parse', '--short=7', 'HEAD')
    banner = ('<b>ArduPlane Stratos %s</b> on ArduPlane %s · README at commit <code>%s</code> · '
              '<a href="%s/blob/%s/README_CUSTOM.md" target="_blank" rel="noopener">source on GitHub</a>'
              % (fork, upstream, commit, GITHUB, branch))

    page = (TEMPLATE.replace('{{TITLE}}', 'ArduPlane Stratos %s README' % fork)
            .replace('{{BANNER}}', banner).replace('{{TOC}}', toc).replace('{{BODY}}', body))
    with open(out, 'w', encoding='utf-8', newline='\n') as f:
        f.write(page)

    ids = set(re.findall(r'id="([^"]+)"', body))
    bad = sorted(set(a for a in re.findall(r'href="#([^"]+)"', page) if a not in ids))
    print('%s: %d bytes, %d headings' % (out, len(page), len(ids)))
    if bad:
        print('links to missing headings: ' + ', '.join(bad))
    return not bad


TEMPLATE = '''<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<title>{{TITLE}}</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Barlow+Semi+Condensed:wght@600;700\
&family=Barlow:ital,wght@0,400;0,500;0,600;1,400&family=JetBrains+Mono:wght@400;600&display=swap">
<style>
/* Sticky outline rail on the left (wide screens), one reading column on the right;
   on phones the outline folds into a list at the top. Same palette as the ArduPilot Tools site. */
:root{--bg:#f2f4f6;--panel:#fff;--ink:#18212b;--muted:#5b6876;--line:#d6dce3;--accent:#a8620a;--code:#e9edf1;--quote:#fbf3e6;
  --f-display:"Barlow Semi Condensed","Arial Narrow",Arial,sans-serif;--f-body:"Barlow","Segoe UI",Roboto,Arial,sans-serif;
  --f-mono:"JetBrains Mono",Consolas,"Courier New",monospace}
@media (prefers-color-scheme: dark){:root:not([data-theme="light"]){--bg:#11161c;--panel:#18202a;--ink:#e4e9ee;--muted:#93a1b0;
  --line:#2b3644;--accent:#e7a33e;--code:#222c38;--quote:#2a2418;color-scheme:dark}}
:root[data-theme="dark"]{--bg:#11161c;--panel:#18202a;--ink:#e4e9ee;--muted:#93a1b0;--line:#2b3644;--accent:#e7a33e;
  --code:#222c38;--quote:#2a2418;color-scheme:dark}
*{box-sizing:border-box}
html{scroll-padding-top:16px}
body{margin:0;background:var(--bg);color:var(--ink);font:16px/1.6 var(--f-body)}
a{color:var(--accent)}
a:focus-visible{outline:2px solid var(--accent);outline-offset:2px}
.shell{max-width:78rem;margin:0 auto;padding-inline:16px;padding-block:32px 64px;display:grid;
  grid-template-columns:17rem minmax(0,1fr);gap:40px;align-items:start}
nav.toc{position:sticky;top:calc(env(safe-area-inset-top,0px) + 16px);max-height:calc(100vh - 32px);overflow:auto;
  font-size:14px;line-height:1.35;padding-right:6px}
nav.toc .label{font:600 12px var(--f-body);letter-spacing:.08em;text-transform:uppercase;color:var(--muted);margin:0 0 10px}
nav.toc ul{list-style:none;margin:0;padding:0;display:grid;gap:4px}
nav.toc ul ul{padding-left:12px;margin:4px 0 10px;border-left:1px solid var(--line)}
nav.toc li.g>a{font-weight:600;color:var(--ink)}
nav.toc a{color:var(--muted);text-decoration:none;display:block;padding:2px 0}
nav.toc a:hover,nav.toc a.on{color:var(--accent)}
details.mtoc{display:none}
main{min-width:0;background:var(--panel);border:1px solid var(--line);border-radius:10px;padding:clamp(18px,4vw,44px)}
.banner{font-size:14px;color:var(--muted);border-bottom:1px solid var(--line);padding-bottom:14px;margin-bottom:8px}
.banner b{color:var(--ink)}
article{max-width:50rem}
h1,h2,h3,h4{font-family:var(--f-display);line-height:1.15;text-wrap:balance;scroll-margin-top:16px}
h1{font-size:clamp(28px,4vw,38px);margin:2.2em 0 .4em;padding-top:.6em;border-top:2px solid var(--line)}
article>h1:first-child{border-top:0;margin-top:.3em;padding-top:0}
h2{font-size:26px;margin:1.9em 0 .4em}
h3{font-size:21px;margin:1.6em 0 .3em}
h4{font-size:18px;margin:1.4em 0 .3em}
p,li{max-width:68ch}
code{font:13.5px var(--f-mono);background:var(--code);padding:.1em .35em;border-radius:4px;overflow-wrap:anywhere}
pre{background:var(--code);padding:14px 16px;border-radius:8px;overflow-x:auto}
pre code{background:none;padding:0;overflow-wrap:normal}
blockquote{margin:1em 0;padding:10px 16px;background:var(--quote);border-left:3px solid var(--accent);
  border-radius:0 6px 6px 0}
blockquote p{margin:.4em 0}
.tw{overflow-x:auto;margin:1em 0}
table{border-collapse:collapse;font-size:14.5px;font-variant-numeric:tabular-nums}
th,td{border:1px solid var(--line);padding:6px 10px;text-align:left;vertical-align:top}
th{background:var(--code);font-weight:600}
img{max-width:100%;height:auto;border-radius:6px;border:1px solid var(--line)}
img.logo{border:0;border-radius:0}
hr{border:0;border-top:1px solid var(--line);margin:2em 0}
.top{position:fixed;right:16px;bottom:calc(env(safe-area-inset-bottom,0px) + 16px);background:var(--panel);
  border:1px solid var(--line);border-radius:999px;padding:8px 14px;font:600 13px var(--f-body);color:var(--ink);
  text-decoration:none}
@media (max-width:900px){
  .shell{grid-template-columns:minmax(0,1fr);gap:16px;padding-block:16px 64px}
  nav.toc{display:none}
  details.mtoc{display:block;background:var(--panel);border:1px solid var(--line);border-radius:10px;
    padding:12px 16px;font-size:15px}
  details.mtoc summary{font-weight:600;cursor:pointer}
  details.mtoc ul{list-style:none;margin:8px 0 0;padding:0;display:grid;gap:4px}
  details.mtoc ul ul{padding-left:12px;margin:4px 0 8px;border-left:1px solid var(--line)}
  details.mtoc a{color:var(--ink);text-decoration:none}
  main{padding:18px 16px}
}
</style>
</head>
<body>
<div class="shell">
  <nav class="toc" aria-label="Outline"><p class="label">Outline</p>{{TOC}}</nav>
  <details class="mtoc"><summary>Outline</summary>{{TOC}}</details>
  <main>
    <div class="banner">{{BANNER}}</div>
    <article>{{BODY}}</article>
  </main>
</div>
<a class="top" href="#">Top</a>
<script>
// Close the phone outline after a jump; highlight the section being read in the rail.
document.querySelectorAll('details.mtoc a').forEach(a =>
  a.addEventListener('click', () => { a.closest('details').open = false; }));
const rail = new Map([...document.querySelectorAll('nav.toc a')].map(a => [a.getAttribute('href').slice(1), a]));
const heads = [...document.querySelectorAll('article h1[id], article h2[id]')].filter(h => rail.has(h.id));
let current = null;
function mark() {
  let h = heads[0];
  for (const x of heads) { if (x.getBoundingClientRect().top < 120) h = x; else break; }
  const a = h && rail.get(h.id);
  if (a === current) return;
  if (current) current.classList.remove('on');
  current = a;
  if (a) { a.classList.add('on'); a.scrollIntoView({block: 'nearest'}); }
}
addEventListener('scroll', mark, {passive: true});
mark();
</script>
</body>
</html>
'''

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Render README_CUSTOM.md as a two-panel HTML page')
    parser.add_argument('output', help='HTML file to write')
    parser.add_argument('--branch', default=None, help='GitHub branch for repository links (default: current branch)')
    args = parser.parse_args()
    ok = build(args.output, args.branch or git('rev-parse', '--abbrev-ref', 'HEAD'))
    raise SystemExit(0 if ok else 1)
