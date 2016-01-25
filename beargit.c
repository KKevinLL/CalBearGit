#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);

  FILE* fbranches = fopen(".beargit/.branches", "w");
  fprintf(fbranches, "%s\n", "master");
  fclose(fbranches);
   
  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  write_string_to_file(".beargit/.current_branch", "master");

  return 0;
}



/* beargit add <filename>
 * 
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

 int beargit_status() {
  int count;
  char line[FILENAME_SIZE];
  FILE* findex = fopen(".beargit/.index", "r");
  fprintf(stdout, "Tracked files:\n\n");
  count = 0;
  while (fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    fprintf(stdout, "  %s\n", line);
    count++;
  }
  fprintf(stdout, "\n%d %s", count, "files total\n");
  fclose(findex);
  return 0;
}

/* beargit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int beargit_rm(const char* filename) {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  int exist = 0;
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) != 0) {
      fprintf(fnewindex, "%s\n", line); 
    }
    else {
      exist = 1;
      }
    }
  fclose(findex);
  fclose(fnewindex);
  if(!exist){
    fprintf(stderr, "%s %s %s\n", "ERROR: File", filename, "not tracked");
    return 1;
  }
  fs_mv(".beargit/.newindex", ".beargit/.index");
  return 0;
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char* go_bears = "GO BEARS!";

int is_commit_msg_ok(const char* msg) {
  int start, a, b;
  start = 0;
  a = 0;
  b = 0;
  while (msg[a] != 0) {
    if (start == 0) {
      if (msg[a] == go_bears[b]) {
        start = 1;
        a++;
        b++;
      } else {
        a++;
      }
    } else {
      if (msg[a] != go_bears[b]) {
        b = 0;
        a++;
        start = 0;
      } else {
        a++;
        b++;
        if (go_bears[b - 1] == '!' && go_bears[b] == '\0') {
          return 1;
        }
      }
    }
  }
  return 0;
}

void next_commit_id_part1(char* commit_id) {
  /* COMPLETE THE REST */
  for(int i = 0; i < COMMIT_ID_BYTES - COMMIT_ID_BRANCH_BYTES; i++){
    if(commit_id[i] == '0'){
      commit_id[i] = '1';
    }
    else if(commit_id[i] == '1'){
      commit_id[i] = '6';
      return;
    }
    else if(commit_id[i] == '6'){
      commit_id[i] = 'c';
      return;
    }
    else {
      commit_id[i] = '1';
    }
  }
}

int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char head[FILENAME_SIZE];
  read_string_from_file(".beargit/.current_branch", head, BRANCHNAME_SIZE);
  if(strlen(head) == 0){
    fprintf(stderr, "%s\n", "ERROR: Need to be on HEAD of a branch to commit");
      return 1;
  }
  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id); 

  /* COMPLETE THE REST */
  char dir[FILENAME_SIZE] = ".beargit/";
  char dir_2[FILENAME_SIZE];
  char dir_3[FILENAME_SIZE];
  char index[FILENAME_SIZE] = ".beargit/.index";
  char prev[FILENAME_SIZE] = ".beargit/.prev";
  sprintf(dir, "%s%s",dir, commit_id);
  strcpy(dir_2, dir);
  fs_mkdir(dir);
  sprintf(dir_2, "%s/.index", dir);
  sprintf(dir_3, "%s/.prev", dir);
  fs_cp(index, dir_2);
  fs_cp(prev, dir_3);
  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    char dst[FILENAME_SIZE];
    sprintf(dst, "%s/%s", dir, line);
    fs_cp(line, dst);
    }
  fclose(findex);
  char dir_4[FILENAME_SIZE];
  sprintf(dir_4, "%s/.msg", dir);
  write_string_to_file(dir_4, msg);
  write_string_to_file(".beargit/.prev", commit_id);
  return 0;
}

/* beargit log
 *
 * See "Step 4" in the homework 1 spec.
 * 
 */

int beargit_log(int limit) {
  /* COMPLETE THE REST */
  char prev_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", prev_id, COMMIT_ID_SIZE);
  if(prev_id[0] == '0'){
    fprintf(stderr, "%s\n", "ERROR: There are no commits!");
    return 1;
  }
  while(limit != 0){
    char dir[FILENAME_SIZE];
    char msg_name[FILENAME_SIZE];
    char msg[MSG_SIZE];
    sprintf(dir, ".beargit/%s/.prev", prev_id);
    sprintf(msg_name, ".beargit/%s/.msg", prev_id);
    read_string_from_file(msg_name, msg, MSG_SIZE);
    fprintf(stdout, "\ncommit %s\n", prev_id);
    fprintf(stdout, "    %s\n", msg);
    read_string_from_file(dir, prev_id, COMMIT_ID_SIZE);
    if(prev_id[0] == '0'){
      fprintf(stdout, "%s", "\n");
      return 0;
    }
    limit--;
  }
  return 0;
}

const char* digits = "61c";

void next_commit_id(char* commit_id) {
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // The first COMMIT_ID_BRANCH_BYTES=10 characters of the commit ID will
  // be used to encode the current branch number. This is necessary to avoid
  // duplicate IDs in different branches, as they can have the same pre-
  // decessor (so next_commit_id has to depend on something else).
  int n = get_branch_number(current_branch);
  for (int i = 0; i < COMMIT_ID_BRANCH_BYTES; i++) {
    commit_id[i] = digits[n%3];
    n /= 3;
  }

  // Use next_commit_id to fill in the rest of the commit ID.
  next_commit_id_part1(commit_id + COMMIT_ID_BRANCH_BYTES);
}


// This helper function returns the branch number for a specific branch, or
// returns -1 if the branch does not exist.
int get_branch_number(const char* branch_name) {
  FILE* fbranches = fopen(".beargit/.branches", "r");

  int branch_index = -1;
  int counter = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      branch_index = counter;
    }
    counter++;
  }

  fclose(fbranches);

  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the homework 1 spec.
 *
 */

int beargit_branch() {
  /* COMPLETE THE REST */
  char cur_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", cur_branch, BRANCHNAME_SIZE);
  char line[BRANCHNAME_SIZE];
  FILE* findex = fopen(".beargit/.branches", "r");
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if(strcmp(line, cur_branch) == 0) {
      fprintf(stdout, "* %s\n", line);
    }
    else{
      fprintf(stdout, "  %s\n", line);
    }
  }
  fclose(findex);
  return 0;
}

/* beargit checkout
 *
 * See "Step 6" in the homework 1 spec.
 *
 */

int checkout_commit(const char* commit_id) {
  /* COMPLETE THE REST */
  char index[FILENAME_SIZE] = ".beargit/.index";
  char dir[FILENAME_SIZE];
  char dir_id[FILENAME_SIZE];
  sprintf(dir_id, ".beargit/%s", commit_id);
  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
      strtok(line, "\n");
      fs_rm(line);
    }
  fs_rm(index);
  fclose(findex);
  if(commit_id[10] != '0') {
    sprintf(dir, ".beargit/%s/.index", commit_id);
    FILE* commit_findex = fopen(dir, "r");
    char name[FILENAME_SIZE];
    while(fgets(name, sizeof(name), commit_findex)) {
      strtok(name, "\n");
      char file[FILENAME_SIZE];
      sprintf(file, "%s/%s", dir_id, name);
      fs_cp(file, name);
    }
    fs_cp(dir, index);
    fclose(commit_findex);
  }
  else {
      FILE* new_index = fopen(".beargit/.index", "w");
      fclose(new_index);
  }
  write_string_to_file(".beargit/.prev", commit_id);

  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  /* COMPLETE THE REST */
  if(strlen(commit_id) != COMMIT_ID_BYTES){
      return 0;
  }
  for(int i = 0; i < strlen(commit_id); i++){
    if(commit_id[i] != '6' && commit_id[i] != '1' && commit_id[i] != 'c'){
      return 0;
    }
  }
  return 1;
}

int beargit_checkout(const char* arg, int new_branch) {
  // Get the current branch
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // If not detached, update the current branch by storing the current HEAD into that branch's file...
  // Even if we cancel later, this is still ok.
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE+50];
    sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

  // Check whether the argument is a commit ID. If yes, we just stay in detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    char commit_dir[FILENAME_SIZE] = ".beargit/";
    strcat(commit_dir, arg);
    if (!fs_check_dir_exists(commit_dir)) {
      fprintf(stderr, "ERROR: Commit %s does not exist\n", arg);
      return 1;
    }

    // Set the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }

  // Just a better name, since we now know the argument is a branch name.
  const char* branch_name = arg;

  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(branch_name) >= 0);

  // Check for errors.
  if (!(!branch_exists || !new_branch)) {
    fprintf(stderr, "ERROR: A branch named %s already exists\n", branch_name);
    return 1;
  } else if (!branch_exists && !new_branch) {
    fprintf(stderr, "ERROR: No branch %s exists\n", branch_name);
    return 1;
  }

  // File for the branch we are changing into.
  char branch_file[FILENAME_SIZE] = ".beargit/.branch_"; 
  strcat(branch_file, branch_name);

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file); 
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // Read the head commit ID of this branch.
  char branch_head_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);

  // Check out the actual commit.
  return checkout_commit(branch_head_commit_id);
}
