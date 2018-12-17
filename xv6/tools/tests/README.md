| part | testing | file | what I changed |
|------|---------|------|----------------|
|1| Inode types | test_badinode.img | invalid inode type - too big|
|2|valid addresses|test_addr_dir.img|direct address too big|
|2|                | test_addr_indir.img | indirect block too big|
|2|                | test_addr_indir2.img | indirect address too big|
|3|root directory|test_badroot.img|parent of root points to 4|
|4|directory formatting|test_baddir.img|took away the "." entry in a directory|
|5|used addresses in bitmap|test_badbitmap.img|edited the bitmap|
|6|bitmap is correct|test_extrabitmap.img|added extra bits to bitmap|
|7|direct addresses used once|test_twicedir.img|add double link to a direct block|
|8|indirect addresses used once|||
|9|all inodes referenced|||
|10|inodes marked in use|||
|11|correct reference counts|||
|12|only one link per directory|||
