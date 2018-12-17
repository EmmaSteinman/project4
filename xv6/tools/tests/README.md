| part |expected output| file | what I changed |
|------|---------|------|----------------|
|1| ERROR: bad inode. | test_badinode.img | invalid inode type - too big|
|2|ERROR: bad direct address in inode.|test_addr_dir.img|direct address too big|
|2|ERROR: bad indirect address in inode.| test_addr_indir.img | indirect block too big|
|2|ERROR: bad indirect address in inode.| test_addr_indir2.img | indirect address too big|
|3|ERROR: root directory does not exist.|test_badroot.img|parent of root points to 4|
|4|ERROR: directory not properly formatted.|test_baddir.img|took away the "." entry in a directory|
|5|ERROR: address used by inode but marked free in bitmapm.|test_badbitmap.img|edited the bitmap|
|6|ERROR: bitmap marks block in use but it is not in use. |test_extrabitmap.img|added extra bits to bitmap|
|7|ERROR: direct address used more than once. |test_twicedir.img|add double link to a direct block|
|9|ERROR: inode marked use but not found in a directory.|test_inodenotfound.img| removed only reference of an inode in directory|
|11|ERROR: bad reference count for file.|test_badlinks.img|changed nlink|
|12|ERROR: directory appears more than once in file system.|test_bad_dir_ref.img|added another reference to directory|
|all|NONE| shin.img| Nothing- this is a consistent file system.|
|all| NONE|test.img| Nothing - this is a consistent file system.|
