# Lab 1 – Custom Shell (`s3`)
By Shashank and Archit

## Testing

![Basic commands](Images/BasicCommands.png)

Testing the commands from the image above

### Task 1 – Basic commands

Testing given commands

→ `whoami`

![whoami](Images/task1_whoami.png)

→ `pwd`

![pwd](Images/task1_pwd.png)

→ `ls`

![ls](Images/task1_ls.png)

→ `ls -R`

![ls -R](Images/task1_ls-R.png)

→ `cat txt/phrases.txt`

![cat txt/phrases.txt](Images/task1_cat_phrases.png)

→ `sort txt/phrases.txt`

![sort txt/phrases.txt](Images/task1_sort_phrases.png)

→ `grep -n BURN txt/phrases.txt`

![grep -n BURN txt/phrases.txt](Images/task1_grep_BURN.png)

→ `cp txt/phrases.txt txt/phrases_copy.txt`

![cp txt/phrases.txt txt/phrases_copy.txt](Images/task1_cp_phrases.png)

→ `wc txt/phrases.txt`

![wc txt/phrases.txt](Images/task1_wc_phrases.png)

→ `uniq txt/phrases.txt`

![uniq txt/phrases.txt](Images/task1_uniq_phrases.png)

→ `touch txt/new_file.txt`

![touch txt/new_file.txt](Images/task1_touch_newfile.png)

→ `chmod a-w txt/phrases_copy.txt`  
→ `ls -l txt/phrases_copy.txt`

![chmod a-w and ls -l txt/phrases_copy.txt](Images/task1_chmod_phrases_copy.png)


![Redirection commands](Images/CommandsWithRedirections.png)

Testing the commands from the image above

### Task 2 – Commands with redirection

Testing given commands

→ `ls > txt/folder_contents.txt`

![ls > txt/folder_contents.txt](Images/task2_ls_to_folder_contents.png)

→ `ls -R > txt/folder_contents.txt`

![ls -R > txt/folder_contents.txt](Images/task2_ls-R_to_folder_contents.png)

→ `echo "=== Full-Year Calendar ===" > txt/calendar.txt`

![echo heading](Images/task2_echo_calendar_heading.png)

→ `cal -y >> txt/calendar.txt`

![cal -y >>](Images/task2_cal_y_append.png)

→ `tac txt/phrases.txt > txt/phrases_reversed.txt`

![tac](Images/task2_tac_phrases.png)

→ `sort txt/phrases.txt > txt/phrases_sorted.txt`

![sort](Images/task2_sort_phrases_sorted.png)

→ `head -n 5 txt/phrases.txt >> txt/phrases_sorted.txt`

![head](Images/task2_head_append.png)

→ `wc txt/phrases.txt > txt/phrases_stats.txt`

![wc](Images/task2_wc_phrases_stats.png)

→ `grep June < txt/calendar.txt`

![grep June](Images/task2_grep_June.png)

→ `tr a-z A-Z < txt/phrases.txt`

![tr](Images/task2_tr_uppercase.png)
















