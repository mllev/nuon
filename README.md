#### nuon
Details coming...
```
CREATE (p:Person {"name": "Matt"}), (q:Person {"name": "Jordan"}), (r:Person {"name": "Matt"}), (q:Person {"name": "Jordan"}), (s:Person {"name": "Matt"}), (t:Person {"name": "Jordan"}), (p:Person {"name": "Matt"}), (u:Person {"name": "Jordan"}), (v:Person {"name": "Matt"}), (w:Person {"name": "Jordan"}), (p) -[:knows]-> (q), (q) -[:hates]-> (t), (t) -[:loves]-> (s)
```