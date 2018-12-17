# Test files

#### Check 1

- **bad_inode.img** - output =ERROR: bad inode.   
  An inode is allocated but an invalid type.
  Inode 16 is of type 5, INVALID.

#### Check 2

- **bad_dir_addr.img** - output = ERROR: bad direct address in inode.   
  An inode contains a direct address greater than the boundary of the file system image

- **bad_dir_addr2.img** - output = ERROR: bad direct address in inode.    
  An inode contains a direct address in the inode region of the file system image
    In the inode at 0x480, inode 2, change the first address to 0x0a(an inode block).

- **bad_indir_addr.img** - output = ERROR: bad indirect address in inode.   
  An inode contains a direct address outside of the file system image
  created a file, file3.txt, that is large enough to require indirect address.
  I then changed the indirect address in the inode, at 0x6c0, to 0xff44.

#### Check 3
- **bad_root_parent.img** - output = ERROR: root directory does not exist.    
  root exists but the parent(..) does not point to itself. Changed to point to inode 2

#### Check 4
- **bad_directory.img** - output = ERROR: directory not properly formatted.   
  A directory does not include ".".
  Changing the second directory inode data, at 0x3c00 so that it does not include "." .

- **dir_unorder.img** - output = NOTHING  This is consistent    
  A directory where "." and ".." are not the first two entries. This test should not cause any error.
  Changing the data in root directory at 0x3a00.

#### Check 5
- **bad_bitmap_free.img** - output = ERROR: address used by inode but marked free in bitmap.    
  An address is in an inode but not in the bitmap. Changed the last 1 bit to a 0 bit in the bitmap.

- **bad_bitmap_free2.img** - output = ERROR: address used by inode but marked free in bitmap.
  Increase the size of an inode and includes a valid addr

#### Check 6
- **bad_bitmap_used.img** - output =ERROR: bitmap marks block in use but it is not in use.    
  A block is marked used in the bitmap but is not used in the File System. add a 1 bit to bitmap

#### Check 7
- **repeat_dir_addr.img** - output = ERROR: direct address used more than once.   
  Change the first address of the 4th indode equal to the first address of the root inode.

#### Check 8
- **repeat_indir_addr.img** - output = ERROR: indirect address used more than once.   
  Two inodes are using indirect addresses, inode at 0x620 and 0x7c0. I changed the indirect address
  of the inode at 0x7c0 to be equal to the indirect address of 0x6c0.

#### Check 9
- **bad_ref_inode2.img** - output = ERROR: inode marked use but not found in a directory.   
Create an inode that is valid but not in a directory. created an empty file at 0x880.

#### Check 10
- **check10.img** - output = ERROR: inode referred to in directory but marked free.   
  added an entry to the root directory pointing to inode 0xaa. inode 0xaa is not in use.

#### Check 11
- **link_counts.img** - output = ERROR: bad reference count for file.   
Increased the nlink value in an inode of a file.

- **link_counts1.img** - output = ERROR: bad reference count for file.      
Decreased the nlink value in an inode of a file.

#### Check 12
- **dir_links.img** - output = ERROR: directory appears more than once in file system.    
Make a directory include another directory. Creating a loop in the system.

#### EC 1: Proper Parent
- **bad_parent1.img** - output = ERROR: parent directory mismatch.    
Change the ".." entry to point to a different directory.

- **bad_parent2.img** - output = ERROR: parent directory mismatch.    
Change the parent directory to not include the original directory.

#### EC 2: Trace to root
- **dir_loop.img** - output = ERROR: inaccessible directory exists.
set the parent to a subdirectory. set folder1 parent to inner1.

- **isolated_dir.img** - output = ERROR: inaccessible directory exists.
Change the parent to itself
