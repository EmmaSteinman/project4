

| file name  | part it tests  | expected result  | notes | pass? |
|------------|----------------|------------------|-------|-------|
| bad_inode_type1.img | 1 | ERROR: bad inode. | Gives an inode an invalid type (one that is not T_DIR, T_FILE, or T_DEV). |yes |
| bad_inode_type2.img | 1 | ERROR: bad inode. | Gives an inode an invalid type (one that is not T_DIR, T_FILE, or T_DEV). |yes|
| bad_inode_type3.img | 1 | ERROR: bad inode. | Gives an inode an invalid type (one that is not T_DIR, T_FILE, or T_DEV). |yes|
| bad_dir_ptr1.img | 2 | ERROR: bad direct address in inode. | Gives an inode a direct address that is too big (outside of the scope of the disk image). |yes|
| bad_dir_ptr2.img | 2 | ERROR: bad direct address in inode. | Gives an inode a direct address that is too small (below the data blocks). |yes|
| bad_ind_ptr1.img | 2 | ERROR: bad indirect address in inode. | Gives an inode a pointer to an indirect block that is too big (outside of the scope of the disk image). |no|
| bad_ind_ptr2.img | 2 | ERROR: bad indirect address in inode. | Gives an inode a pointer to an indirect block that is too small (below the data blocks). | no|
| bad_ind_ptr3.img | 2 | ERROR: bad indirect address in inode. | Puts an indirect pointer in an indirect pointer block that is too small (below the data blocks). |no|
| bad_ind_ptr4.img | 2 | ERROR: bad indirect address in inode. | Puts an indirect pointer in an indirect pointer block that is too big (outside of the scope of the disk image). | yes|
| bad_root1.img | 3 | ERROR: root directory does not exist. | Sets the root inode's type to 0 to mark it unused. |yes|
| bad_root2.img | 3 | ERROR: root directory does not exist. | Changes the name of the root directory's parent entry. |yes|
| bad_root3.img | 3 | ERROR: root directory does not exist. | Changes the inode number of the root directory's parent entry. |yes|
| bad_dir_format1.img | 4 | ERROR: directory not properly formatted. | Sets the data block of a directory to be completely empty. |yes|
| bad_dir_format2.img | 4 | ERROR: directory not properly formatted. | Associates a directory's . entry with the wrong inode number. |yes|
| bad_dir_format3.img | 4 | ERROR: directory not properly formatted. |  Removes the . entry from a directory. |yes|
| bad_dir_format4.img | 4 OR extra credit 1 | ERROR: directory not properly formatted. OR ERROR: parent directory mismatch (if extra credit part 1 has been implemented) | Removes the .. entry from a directory. |yes|
| bad_inode_addr1.img | 5 | ERROR: address used by inode but marked free in bitmap. | Points a direct address in an inode to an unused block. |yes|
| bad_inode_addr2.img | 5 | ERROR: address used by inode but marked free in bitmap. | Points an indirect address in an inode to an unused block. |yes|
| bad_bitmap1.img | 6 | ERROR: bitmap marks block in use but it is not in use. | Removes a direct pointer from an inode but keeps it marked as used in the bitmap. |no|
| bad_bitmap2.img | 6 | ERROR: bitmap marks block in use but it is not in use. | Removes an indirect pointer from an inode but keeps it marked as used in the bitmap. |no|
| reused_dir_ptr1.img | 7 | ERROR: direct address used more than once. | Tests a situation where the same inode contains two pointers to the same address. |yes|
| reused_dir_ptr2.img | 7 | ERROR: direct address used more than once. | Tests a situation where different inodes contain pointers to the same address. |yes|
| reused_ind_ptr1.img | 8 | ERROR: indirect address used more than once. | Tests a situation where one inode's block of indirect pointers contains the same pointer twice. |yes|
| reused_ind_ptr2.img | 8 | ERROR: indirect address used more than once. | Tests a situation where different inodes' indirect blocks contain the same pointer. |yes|
| bad_mark_use1.img | 9 | ERROR: inode marked use but not found in a directory. | Places a new inode that is never referred to in a directory in the lowest-possible free slot in the inode blocks. | no|
| bad_mark_use2.img | 9 | ERROR: inode marked use but not found in a directory. | Places a new inode that is never referred to in a directory in a higher free slot in the inode blocks.|no|
| bad_dir_use1.img | 10 | ERROR: inode referred to in directory but marked free. | Places a reference to a non-existent inode in the direct block of a directory. |yes|
| bad_dir_use2.img | 10 | ERROR: inode referred to in directory but marked free. | Places a reference to a non-existent inode in the indirect block of a directory. |yes|
| ind_dir1.img | n/a | no error message | Adds an indirect pointer to a directory to check that this doesn't cause any problems. SHOULD PASS WITH NO ERRORS - THIS IS A CONSISTENT FILE SYSTEM. |yes|
| ind_dir2.img | n/a | no error message | Adds multiple indirect pointers to a directory's indirect block to check that this doesn't cause any problems. SHOULD PASS WITH NO ERRORS - THIS IS A CONSISTENT FILE SYSTEM. |yes|
| bad_link_count1.img | 11 | ERROR: bad reference count for file. | Sets a regular file's inode link count in its inode struct to fewer than the number of times it is actually referred to. |yes|
| bad_link_count2.img | 11 | ERROR: bad reference count for file. | Sets a regular file's inode link count in its inode struct to more than the number of times it is actually referred to. |yes|
| bad_link_count3.img | 11 | ERROR: bad reference count for file. | Adds an extra hard link to a file by adding a directory entry with its inode number without updating its inode struct link count. |yes|
| link_count_ok1.img | 11 | no error message | Creates a new hard link between two files, but all necessary structures have been updated correctly. SHOULD PASS WITH NO ERRORS - THIS IS A CONSISTENT FILE SYSTEM. |yes|
| bad_dir_ref1.img | 12 | ERROR: directory appears more than once in file system. | Places multiple entries for the same directory in the direct addressed blocks of its parent. |yes|
| bad_dir_ref2.img | 12 | ERROR: directory appears more than once in file system. | Places multiple entries for the same directory in the direct addressed blocks of different parent directories. |yes|
| bad_dir_ref3.img | 12 | ERROR: directory appears more than once in file system. | Places multiple entries for the same directory in the block of indirect addresses for its parent directory. |yes|
| bad_parent1.img | extra credit 1 | ERROR: parent directory mismatch. | Sets the inode number for the parent directory of a directory to the wrong number. |
| bad_parent2.img | extra credit 1 | ERROR: parent directory mismatch. | The inode number of a directory's parent is correct, but the parent does not contain an entry for the child.
