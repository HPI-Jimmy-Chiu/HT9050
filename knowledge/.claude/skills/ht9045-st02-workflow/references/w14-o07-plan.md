# W14 (O07): test-timeout Reset clears parts and rewrites the recipe's Contact.Data?

> Status 20260927: **read-only first**, then propose.
> Steven leans toward: **only change the in-use TestIF variables, don't write the recipe file**
> (testif = in use, testif_file = read from the file).

## Background (from progress-st02 #14)
- Golden (atester.cpp:3860-3871, Sam 20250820 [I49]): on a test time-out with Reset / clear parts, it rewrites the recipe
  Contact.Data: [Mode] Contact → Direct, and [Test Arm1] / [Test Arm2] Contact raised by I49.

## To do
1. Read golden 912 `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\atester.cpp:3860-3871` and `csystem.cpp:3106-3117`,
   plus the 906_0625_Steven equivalents: does it really WRITE Contact.Data (which function, which keys), or only memory?
2. Find the V906 site (atester.cpp / csystem.cpp; gate id) and what TestIF vs TestIF_File hold for those fields.
3. Propose the in-memory approach: change only the in-use TestIF (and whatever the contact logic reads), never
   WriteIniData to Contact.Data; say what resets it (next recipe load?) and what the operator sees.
4. Send the plan to github-46; no code until GO.
