const char stonymask1[112*112]={92,85,65,79,83,83,83,70,75,66,85,76,80,72,90,88,83,95,95,75,77,74,80,78,82,65,56,100,87,93,69,71,117,77,90,82,80,88,91,74,100,71,90,97,92,86,82,44,73,81,81,91,88,74,73,66,85,89,86,80,70,66,81,77,63,87,81,80,70,103,86,75,97,58,75,88,80,70,81,86,88,69,83,86,98,83,81,66,74,77,59,95,86,88,92,69,87,74,98,92,90,97,90,82,80,61,62,81,85,80,69,84,44,36,50,53,54,50,58,47,31,38,50,44,41,32,51,59,54,44,56,55,52,62,43,68,61,57,59,65,53,73,49,38,72,46,41,47,56,48,65,46,72,52,49,61,62,52,64,52,45,44,53,36,48,54,69,57,54,48,58,58,45,59,42,41,58,51,39,35,54,60,68,51,28,36,48,65,65,63,54,61,66,44,67,63,58,54,39,49,59,61,36,56,57,54,60,46,41,55,55,51,48,45,45,51,61,49,49,52,49,55,56,53,48,60,52,68,62,45,56,51,52,62,61,68,16,52,52,49,48,54,58,59,59,41,53,63,51,46,45,53,52,49,49,40,67,57,45,70,62,41,52,51,56,61,49,57,59,69,37,46,39,54,46,53,38,45,48,63,71,42,49,58,57,58,53,44,56,52,57,56,49,74,42,46,29,62,60,76,73,45,38,60,48,53,58,46,65,65,43,45,65,52,39,36,58,60,47,30,57,67,59,35,42,56,61,52,52,62,60,39,27,49,63,70,67,68,58,43,67,39,61,52,74,60,48,44,28,44,47,52,47,39,60,69,65,41,36,41,69,59,64,57,56,54,57,60,67,47,38,61,44,51,54,53,58,46,45,59,57,65,59,44,48,37,54,55,43,59,52,50,68,43,38,63,46,48,67,37,59,59,49,53,58,71,46,43,41,44,54,67,45,52,40,74,52,14,70,57,67,60,47,55,54,49,45,58,43,47,56,35,47,68,59,55,68,60,48,57,61,61,51,36,42,33,59,62,69,68,76,73,55,50,64,34,62,36,55,63,63,43,62,54,41,38,49,47,47,57,51,51,71,35,65,56,76,46,38,46,63,61,39,65,49,45,63,44,65,52,57,52,56,41,43,32,25,51,57,50,61,46,48,63,57,66,51,61,50,64,33,44,59,60,61,45,56,66,64,54,35,52,59,50,66,39,39,59,36,51,65,35,79,67,52,45,56,53,55,56,57,49,56,57,69,48,55,56,40,61,56,49,52,56,62,60,50,48,57,54,51,43,58,58,59,50,70,52,34,56,46,59,46,45,60,58,59,60,44,34,67,49,54,63,79,49,52,54,54,52,55,58,59,60,20,66,59,41,58,61,60,47,63,59,49,54,59,55,58,63,50,58,54,39,67,54,58,44,48,60,52,28,50,35,49,66,48,43,54,60,54,35,39,36,31,72,56,55,48,58,36,48,57,46,48,29,60,32,55,50,46,73,49,57,40,53,68,71,58,51,48,53,73,57,62,58,72,59,39,44,52,66,49,57,49,60,64,47,60,53,54,44,47,61,46,44,62,59,59,54,75,46,63,63,52,41,57,55,72,66,48,48,34,44,69,46,39,41,58,61,61,38,51,56,68,51,59,35,52,43,38,64,57,64,54,48,66,61,52,47,49,45,61,64,52,39,48,42,68,48,63,56,59,33,42,53,39,62,49,52,41,43,55,56,57,33,67,56,49,59,42,49,45,67,61,50,76,51,40,57,53,43,57,56,67,50,60,47,45,49,47,57,53,41,60,71,55,54,55,56,43,41,49,36,52,61,49,39,59,46,47,59,57,53,69,53,63,48,79,54,44,60,61,53,31,50,66,61,51,56,68,58,68,49,45,72,69,63,53,69,49,39,37,46,47,48,58,56,48,67,57,27,57,60,63,64,49,43,56,45,46,34,55,54,66,65,58,38,41,58,50,27,64,51,64,34,49,69,51,54,46,37,60,50,50,61,52,41,60,34,40,61,71,51,52,55,78,60,58,47,61,68,36,59,45,49,57,62,70,61,54,52,57,42,61,41,52,52,47,42,63,53,46,50,41,70,57,45,54,63,69,51,47,59,34,51,42,43,59,64,53,58,49,43,46,54,46,64,52,59,45,68,58,33,58,65,55,51,42,54,59,55,61,54,50,36,39,53,47,45,43,39,63,44,50,68,38,50,24,23,55,74,43,44,34,51,51,67,65,63,61,56,26,34,55,50,54,57,48,63,52,42,36,59,61,39,54,47,67,44,75,55,25,40,46,31,61,65,48,69,55,52,62,55,64,30,53,52,68,51,36,46,50,56,40,57,66,55,53,47,68,61,60,61,60,55,56,46,46,53,70,44,60,52,46,60,69,55,30,61,57,48,62,69,43,45,41,55,54,38,46,47,36,45,53,46,61,48,47,48,57,42,69,50,58,36,44,58,38,75,50,53,64,28,39,45,70,51,53,47,58,43,43,37,35,39,56,50,53,79,58,43,49,35,54,59,47,54,45,67,59,63,44,50,72,54,48,52,57,57,63,38,56,58,60,57,41,47,73,62,67,48,43,43,49,44,31,72,44,58,60,51,55,58,64,41,55,51,60,64,56,59,70,58,68,58,56,28,70,59,67,47,46,47,72,49,43,47,40,44,42,49,52,57,60,63,60,52,54,48,46,61,56,38,43,55,44,33,28,76,67,48,43,45,69,69,60,54,46,68,55,46,53,32,61,42,56,46,57,41,60,55,48,54,63,58,45,63,54,55,43,59,52,55,55,49,38,51,46,64,61,75,55,68,55,59,42,38,63,28,56,60,43,55,55,40,57,52,46,61,74,47,63,56,50,53,72,57,46,62,52,47,54,54,69,51,51,57,57,46,45,46,63,73,66,55,58,40,62,45,54,45,40,59,53,52,58,49,60,42,63,54,40,45,27,46,55,58,42,49,39,69,76,48,44,38,50,61,67,46,53,54,57,54,69,48,65,46,51,46,62,45,57,61,55,54,59,39,37,70,51,56,40,38,38,66,56,52,38,58,56,57,54,74,60,48,68,49,64,47,65,42,45,48,66,63,30,48,44,63,37,48,56,64,58,58,56,52,70,46,30,52,52,59,62,44,64,42,44,53,52,53,65,57,56,61,54,30,62,65,47,63,60,66,58,73,60,47,59,56,49,51,67,50,52,63,49,41,51,56,62,52,34,51,59,49,50,35,71,70,67,56,60,47,30,53,51,59,76,57,36,45,46,70,71,68,68,55,67,51,39,60,46,63,66,44,62,49,58,44,50,69,83,43,52,75,57,44,54,54,52,47,48,38,61,32,53,55,44,52,58,46,58,66,62,46,54,47,54,61,56,67,60,40,60,54,68,63,72,40,57,40,49,45,73,47,67,65,64,52,65,59,49,23,24,41,61,57,60,45,58,67,36,55,55,41,37,49,49,34,50,37,49,44,56,58,41,52,61,40,68,46,45,53,51,43,61,51,42,51,74,55,47,49,47,51,41,51,55,49,44,55,41,71,63,59,54,59,64,44,57,45,70,44,38,67,51,68,40,62,56,53,70,39,54,56,47,43,64,66,57,38,33,58,30,54,56,29,64,55,61,46,59,51,69,63,53,61,29,49,83,46,60,58,63,58,56,50,70,51,66,49,53,55,56,37,33,47,48,41,40,28,60,52,44,47,62,56,46,60,36,45,48,58,49,56,61,68,43,52,50,55,51,60,58,55,54,52,54,46,65,53,54,54,42,46,55,39,45,58,49,32,61,40,54,64,49,53,56,57,51,52,32,54,36,45,55,28,51,48,61,53,62,58,61,40,64,35,60,61,58,28,63,33,72,56,50,50,51,50,51,62,56,53,81,50,44,44,45,50,57,41,83,66,56,47,48,60,65,53,56,57,62,48,59,59,45,53,63,45,42,59,53,32,39,72,75,56,50,25,57,37,52,54,53,55,45,52,51,55,57,50,46,49,51,70,70,27,60,53,48,52,52,41,61,69,47,53,51,54,52,26,68,37,64,44,49,65,71,34,43,58,52,51,41,64,46,48,58,62,53,44,68,64,62,50,47,32,72,35,68,58,70,53,59,47,55,46,52,36,60,58,65,59,63,44,65,42,51,62,57,56,38,55,49,46,68,48,57,47,68,45,59,67,56,48,52,50,64,33,40,51,47,55,68,53,66,69,50,48,51,55,53,52,52,40,56,66,58,40,50,61,52,52,49,40,47,70,51,59,45,55,57,34,58,51,72,61,49,33,60,52,36,50,41,45,53,29,48,48,49,55,65,59,48,53,42,53,53,57,67,57,55,56,65,63,62,60,58,41,41,43,57,50,60,50,49,18,58,47,58,52,48,51,75,50,52,51,58,49,53,52,70,55,51,52,63,61,49,37,36,50,61,41,76,68,44,60,60,55,57,50,34,65,52,46,28,49,55,71,46,39,60,49,57,56,50,58,54,36,41,52,56,58,57,62,33,46,35,59,51,45,43,56,55,51,52,36,63,41,41,38,60,52,45,45,56,47,60,45,44,52,52,62,46,41,64,45,56,46,49,57,54,61,65,59,62,66,58,41,47,33,67,46,41,55,64,61,51,39,62,33,48,59,43,53,58,57,60,55,51,52,43,46,61,59,43,47,64,66,41,47,64,57,63,48,67,41,60,46,62,57,51,38,52,45,33,48,38,50,59,60,54,50,56,24,45,61,41,46,58,47,49,39,44,44,44,54,33,35,55,64,54,23,47,69,48,34,78,51,59,53,48,34,49,34,25,58,52,49,47,50,59,57,56,53,48,56,40,55,46,47,57,51,55,38,57,51,57,58,45,52,55,33,70,54,53,53,69,64,51,41,55,58,62,54,62,45,49,49,33,47,71,36,56,32,37,52,52,68,71,42,38,57,54,27,46,61,69,55,74,64,67,42,34,39,55,55,48,46,65,38,44,47,33,47,69,54,39,44,29,65,57,46,54,43,63,46,54,41,38,55,44,58,40,48,36,58,45,40,53,56,64,41,40,44,52,47,45,58,50,31,63,50,53,41,67,47,28,52,69,40,67,52,57,58,45,39,53,66,41,65,52,68,62,26,62,43,51,47,54,43,60,37,51,54,26,64,45,60,53,41,53,55,54,34,45,48,47,49,42,40,68,62,52,57,57,53,64,35,51,54,50,47,58,69,50,43,49,46,46,40,51,40,67,51,38,64,66,53,56,52,31,49,50,32,72,49,7,58,54,55,43,35,46,59,55,51,67,61,45,50,57,56,43,57,33,49,55,49,62,63,32,61,44,40,55,51,60,54,67,70,48,35,65,29,31,39,39,56,55,46,60,51,54,64,62,54,64,40,57,62,64,59,45,48,46,32,42,54,66,44,56,54,58,67,63,56,40,30,67,47,47,48,57,42,61,64,47,50,76,38,56,59,63,53,60,56,41,17,43,49,40,56,46,36,59,66,52,56,69,45,49,51,59,60,58,64,49,64,52,57,60,55,53,55,41,73,55,58,25,41,50,27,60,36,72,51,52,33,65,43,55,67,35,60,65,31,51,52,57,49,48,54,47,44,52,57,47,45,54,57,59,60,56,63,58,48,69,46,57,47,53,49,68,48,37,36,62,44,53,54,55,52,59,45,39,56,57,36,57,49,48,50,37,42,50,43,52,58,25,37,60,35,58,49,40,32,35,49,36,48,29,50,43,65,46,56,47,62,52,42,49,53,54,69,47,58,43,53,48,39,51,50,50,56,29,54,47,45,57,56,58,50,52,47,58,45,51,58,41,55,56,50,40,40,51,56,52,39,48,43,52,50,40,49,55,44,69,49,54,52,49,34,61,42,78,57,53,50,49,54,63,58,40,56,64,53,46,44,54,34,38,50,55,51,58,47,47,62,61,47,59,53,49,62,33,34,72,49,51,44,61,56,65,41,60,60,48,53,70,52,51,27,58,43,38,54,38,38,51,55,56,47,52,41,36,41,50,57,49,55,44,67,53,60,49,44,47,50,36,52,47,50,48,64,69,54,40,40,78,44,48,48,54,39,63,61,53,34,36,69,58,70,54,44,53,53,54,47,60,44,56,43,61,48,67,46,58,56,78,51,61,63,60,55,56,52,52,55,49,35,67,59,53,55,65,59,51,37,57,55,67,40,67,37,65,49,60,63,59,64,54,52,63,52,53,48,52,65,43,50,60,49,45,48,52,51,41,49,54,47,56,38,58,46,60,49,39,58,43,59,58,53,36,54,37,54,39,24,43,43,49,51,55,48,61,53,46,56,67,45,80,46,42,54,75,44,37,63,53,61,56,42,55,62,55,47,43,45,60,51,52,44,42,34,46,58,44,56,56,35,56,48,59,53,63,53,63,48,45,73,52,54,52,64,42,38,45,54,44,56,45,36,68,45,42,68,47,60,50,52,68,61,54,52,61,43,76,41,49,62,57,54,69,70,51,39,56,54,49,32,61,49,47,65,48,52,76,50,48,40,56,41,54,63,62,45,56,65,51,53,41,53,56,79,46,31,54,59,42,52,66,45,61,48,14,53,45,43,60,59,58,49,74,48,66,65,46,60,53,44,56,48,49,49,63,46,29,57,49,59,53,48,45,40,52,49,64,45,72,44,82,55,47,36,40,40,77,50,56,60,55,56,54,50,83,50,51,77,56,54,50,43,69,55,56,48,37,55,67,52,58,46,44,38,54,53,48,30,46,54,42,59,54,57,65,59,39,41,51,41,45,60,57,58,38,49,55,38,66,50,54,46,51,45,44,53,51,52,38,53,60,44,66,38,57,46,44,30,70,53,44,35,46,46,53,42,56,67,56,59,34,54,45,49,39,30,48,43,51,53,68,34,50,43,54,38,46,44,75,54,34,58,51,44,48,39,51,46,65,46,49,55,59,43,50,49,47,39,55,62,54,75,44,57,74,48,60,58,43,62,61,51,39,75,51,57,58,49,56,26,37,43,59,61,47,46,65,42,59,30,57,39,51,67,56,49,48,49,50,41,52,81,61,38,39,51,41,77,53,49,65,59,54,53,57,53,57,61,60,47,53,56,59,46,63,61,64,58,50,72,48,49,40,61,35,57,37,45,64,46,58,53,59,58,68,54,52,65,42,58,68,58,54,35,41,63,21,59,52,46,54,54,54,61,70,56,52,57,60,60,52,54,48,54,40,42,64,61,67,55,55,58,69,63,73,58,46,60,62,62,41,21,54,40,42,57,70,43,67,64,70,50,50,46,47,68,51,55,49,30,48,34,50,63,66,50,41,50,43,81,55,50,68,69,64,53,39,50,62,64,47,27,60,56,56,54,50,54,66,48,30,65,59,47,52,36,57,45,42,48,52,45,50,57,61,55,71,48,56,51,65,39,64,50,58,19,65,60,49,41,60,35,49,52,48,38,52,41,46,63,66,41,51,65,56,68,59,38,88,49,47,63,33,53,64,52,58,58,53,60,48,44,59,34,60,64,67,52,60,52,54,60,61,50,70,56,16,73,47,46,45,62,47,27,42,69,55,36,52,50,49,60,53,63,58,47,46,58,66,52,72,62,52,43,71,54,57,48,55,47,44,45,52,44,39,56,52,51,44,60,45,43,56,59,55,46,58,65,60,43,61,59,61,41,64,52,52,50,56,62,42,35,58,43,24,45,56,33,57,55,58,63,69,56,51,36,54,63,45,56,80,36,58,37,61,41,62,58,44,52,51,57,57,58,57,16,52,53,56,57,54,31,45,54,62,29,61,45,50,57,57,49,54,49,35,38,52,55,47,37,37,45,54,70,72,47,50,51,50,52,62,37,68,62,46,12,50,70,59,53,51,71,63,47,53,50,55,58,39,54,44,64,49,61,54,35,57,57,55,58,57,62,67,59,56,55,64,41,76,53,55,68,39,48,68,51,59,55,63,55,54,57,66,59,54,43,39,55,57,55,43,36,59,49,52,51,51,36,51,36,69,43,62,45,66,54,59,28,58,48,39,62,61,32,60,57,72,48,56,51,45,54,43,37,56,62,57,56,32,61,58,42,30,49,45,38,42,27,57,47,69,38,76,54,64,63,47,43,61,54,50,59,47,40,56,51,55,79,69,51,53,54,50,53,57,62,54,53,56,55,68,66,66,55,49,61,39,47,42,46,57,59,61,61,52,48,65,42,40,58,58,50,38,55,65,55,69,72,48,50,56,66,48,52,69,57,60,60,42,54,57,38,58,55,43,54,66,70,50,59,43,55,42,51,56,42,61,70,55,64,62,47,48,51,53,52,44,40,60,44,34,77,51,58,37,40,45,65,58,56,50,58,61,58,56,63,62,61,53,45,49,50,57,51,48,37,75,48,65,64,68,50,63,75,72,51,45,54,62,46,59,49,57,60,70,67,54,47,51,45,56,57,54,57,61,75,56,36,57,29,59,40,42,56,41,41,46,61,60,52,62,36,56,54,55,39,66,68,47,45,59,55,62,44,53,55,68,57,55,52,62,25,48,51,57,61,59,41,61,66,60,61,66,59,34,59,55,57,37,59,39,37,40,79,56,47,42,42,71,64,57,54,66,47,40,52,54,57,80,32,40,29,44,49,47,56,46,43,61,38,49,75,66,48,65,60,59,57,61,49,45,60,53,47,52,61,65,64,57,63,47,60,57,41,53,43,46,61,42,42,44,39,44,55,49,38,69,39,47,63,60,42,41,0,66,61,50,49,75,35,54,55,43,56,79,38,66,64,61,56,66,70,55,36,41,49,52,48,47,58,56,59,66,38,65,53,53,55,39,71,36,47,40,35,42,70,46,54,45,53,27,59,56,54,70,39,46,57,57,49,52,32,63,31,55,47,46,59,49,54,65,50,44,51,62,45,29,60,44,52,45,37,45,54,41,39,68,66,53,66,61,46,66,52,51,34,50,69,51,44,31,41,71,48,40,56,65,58,48,46,39,56,69,40,43,55,62,56,36,54,81,45,38,47,57,61,55,56,55,38,46,58,65,48,49,38,49,38,51,47,43,60,56,57,72,48,46,41,32,66,49,43,43,54,63,54,49,62,64,35,32,52,45,66,42,44,32,62,42,54,60,59,70,60,49,42,54,31,47,43,64,64,51,46,49,61,56,67,40,51,56,41,64,50,55,43,44,40,56,60,56,47,60,37,53,36,57,40,58,62,47,41,37,44,53,47,59,66,73,26,66,50,50,61,56,50,57,74,41,57,49,47,43,56,50,39,43,71,55,59,55,54,61,49,47,61,48,26,44,51,58,65,37,59,52,65,69,40,55,47,39,66,48,60,53,68,60,51,37,67,67,48,35,48,53,75,47,51,50,60,47,56,63,48,65,56,49,40,52,38,53,44,44,51,54,38,47,67,51,59,60,71,37,51,46,56,55,45,53,41,35,53,41,72,61,47,59,46,53,34,49,48,65,66,48,33,42,55,45,53,67,49,71,64,56,69,76,43,45,60,55,55,33,56,71,41,58,29,61,52,61,46,65,62,59,48,54,54,43,46,48,47,40,53,65,54,55,39,58,61,59,52,52,35,51,55,53,49,67,43,51,55,54,44,61,46,42,61,57,42,65,61,43,66,64,56,78,62,38,25,49,50,36,61,46,60,62,51,42,62,51,60,35,41,57,62,56,49,25,64,73,65,58,42,37,50,52,32,47,45,54,53,63,64,46,57,52,40,44,56,57,31,50,41,49,51,46,60,69,47,56,58,62,41,47,41,77,44,56,49,57,39,63,46,45,55,63,56,65,41,53,27,31,37,63,61,65,54,54,75,54,48,57,44,67,55,35,50,54,53,61,42,36,63,68,29,46,34,51,68,47,64,54,45,45,46,48,50,43,57,53,52,62,32,57,43,45,40,64,51,43,47,55,58,49,52,61,46,54,35,64,41,49,52,55,53,58,60,62,48,64,44,71,57,43,60,60,53,35,35,35,54,55,43,46,50,53,63,70,66,64,53,55,68,73,43,49,63,76,49,49,53,58,58,68,54,45,52,56,54,53,52,69,35,41,54,47,63,55,57,45,54,65,52,62,45,53,48,47,43,43,58,54,43,39,61,46,40,42,50,44,71,46,50,46,33,33,49,47,44,60,53,41,40,52,51,52,59,62,60,47,69,53,71,66,55,28,47,59,58,45,53,44,40,65,60,56,70,68,69,69,37,48,57,73,46,35,55,61,74,40,44,52,69,45,38,51,71,55,58,46,68,36,55,46,70,63,62,35,37,77,52,53,59,41,62,62,63,50,53,57,52,42,47,60,25,55,61,55,52,69,62,57,58,62,44,57,30,28,46,51,48,50,57,51,46,49,63,58,34,42,56,42,60,54,37,62,52,58,48,56,52,68,57,49,51,70,42,55,51,66,57,68,48,63,61,63,51,39,74,56,48,65,53,55,65,45,62,51,66,44,54,68,52,66,51,63,50,20,65,59,62,43,48,64,44,31,50,54,46,65,45,50,55,74,29,65,61,61,42,59,35,58,53,49,40,62,66,51,48,72,63,66,46,59,46,53,49,41,38,59,60,57,38,52,51,59,40,73,46,56,57,56,38,62,46,69,47,45,60,67,34,24,46,46,68,56,53,67,76,57,44,53,43,72,61,45,52,48,40,45,73,67,64,58,65,51,53,56,53,49,49,57,44,65,42,61,38,63,41,43,56,51,68,56,60,63,24,72,48,43,58,65,61,34,32,62,36,37,51,51,52,52,39,48,60,66,37,38,67,59,67,67,55,79,69,61,31,56,50,69,56,51,53,58,56,49,43,51,47,35,48,62,54,52,56,45,54,62,55,40,33,53,45,59,45,25,53,35,44,50,55,59,54,52,16,46,62,39,48,43,66,47,58,57,49,51,48,40,37,48,50,36,61,43,65,60,44,42,62,53,40,34,57,62,31,40,53,52,60,53,58,71,52,58,53,54,56,61,44,49,48,54,56,72,74,31,27,38,53,76,27,57,53,62,53,53,51,68,42,44,33,58,69,32,59,67,47,54,38,65,58,58,57,43,50,63,38,48,46,46,13,58,49,56,64,66,56,47,64,61,50,50,46,43,41,43,63,55,44,45,40,51,70,38,46,57,41,57,72,60,61,47,55,58,60,58,38,73,49,45,52,47,61,44,58,52,47,58,42,37,71,52,61,53,50,43,51,47,71,52,33,55,53,57,44,57,56,48,49,41,55,65,54,54,42,57,45,46,36,48,54,42,48,51,57,55,54,52,68,73,59,39,59,67,74,45,43,65,57,60,65,49,65,71,31,49,50,62,53,73,65,52,47,45,51,48,65,53,57,49,64,61,58,68,39,49,56,47,44,70,76,43,53,41,61,53,42,29,31,37,62,45,56,41,41,39,53,61,28,63,57,40,26,42,55,46,57,52,59,62,61,66,66,49,53,46,39,48,61,59,40,57,47,56,63,54,58,55,66,72,54,52,40,71,52,45,69,58,58,56,47,59,65,46,49,50,58,64,46,63,69,66,30,56,65,69,44,47,66,83,56,54,39,55,65,55,45,49,49,68,59,64,61,60,26,46,58,43,59,47,62,46,68,65,61,70,45,63,56,62,49,51,53,51,36,51,56,53,62,57,49,49,56,59,52,55,47,60,38,49,37,58,35,48,43,46,41,38,54,66,61,68,55,58,58,51,51,63,44,26,61,40,56,53,56,36,58,18,56,35,41,71,55,66,38,54,56,57,58,47,48,56,31,50,49,36,53,41,44,55,58,53,58,59,70,52,62,59,54,57,38,66,64,40,54,43,33,58,59,53,53,45,66,57,43,43,50,63,51,68,68,53,67,41,70,49,57,38,42,43,65,56,64,49,64,45,42,53,49,66,30,53,24,52,51,68,43,44,49,55,46,66,33,70,46,50,57,66,44,40,45,50,57,55,42,57,56,65,51,42,61,34,64,57,54,60,42,57,59,54,66,60,56,77,47,67,54,50,56,44,73,69,63,44,44,62,49,35,51,70,43,47,61,51,64,57,47,61,52,51,63,59,50,63,62,48,52,39,45,67,44,70,42,54,48,67,68,59,57,41,64,58,55,53,45,46,72,59,51,61,34,51,74,39,58,56,38,39,54,49,66,54,40,37,45,47,62,54,38,55,48,59,51,67,50,66,41,53,48,65,43,64,60,63,56,41,52,44,75,57,46,39,41,55,45,51,38,67,54,62,53,55,65,47,58,59,48,48,55,67,34,54,52,54,83,66,38,63,34,47,45,43,30,63,68,56,54,62,46,47,58,51,65,59,59,65,52,76,58,45,58,75,36,67,61,65,70,52,73,55,37,44,60,38,51,57,47,54,66,56,64,64,37,39,59,44,70,45,58,65,31,55,60,38,52,27,31,50,56,31,79,39,32,64,63,55,37,81,45,46,49,42,42,53,58,51,43,48,62,46,57,58,40,43,62,46,58,55,39,55,63,56,45,70,64,62,49,61,44,66,42,70,66,48,62,63,49,67,51,46,46,60,39,51,62,69,47,58,60,62,49,54,57,68,48,50,47,69,60,57,48,60,48,74,49,38,56,63,42,69,62,57,34,55,46,55,49,63,55,59,76,56,55,67,41,53,47,52,41,42,61,43,55,48,75,50,39,45,33,19,82,53,51,34,54,48,51,61,54,65,47,40,57,52,30,50,52,52,60,64,49,57,71,68,39,42,51,52,42,42,46,67,29,57,59,46,72,65,54,64,55,45,43,57,49,53,46,57,61,58,42,45,47,52,54,52,46,66,57,56,41,64,60,57,56,57,65,41,24,58,62,56,45,50,62,71,48,52,59,47,55,59,47,49,40,32,45,56,48,52,38,27,69,30,62,61,57,48,65,45,56,48,43,66,43,52,66,62,40,56,51,30,50,57,46,54,54,62,43,62,45,65,48,54,44,53,50,69,48,59,55,53,38,66,72,63,48,58,54,46,58,57,47,57,58,49,51,65,51,58,69,49,35,60,27,73,48,69,65,53,67,52,49,61,55,53,47,53,45,47,54,55,73,47,44,58,35,67,48,44,54,67,55,64,50,51,51,49,50,72,53,54,50,53,70,64,41,44,45,50,54,60,62,76,74,52,44,57,39,50,56,38,53,52,52,54,34,53,62,68,40,55,48,60,53,45,61,61,40,45,56,62,51,70,45,48,50,64,44,43,59,29,46,55,54,47,73,50,58,45,47,59,58,63,32,52,42,67,58,47,49,54,59,62,52,45,48,68,39,53,36,64,46,51,35,64,38,53,62,52,55,58,60,55,56,74,50,65,54,60,65,65,38,59,47,45,37,38,35,54,58,58,51,65,61,42,39,52,39,64,61,62,73,56,59,56,72,43,33,69,60,57,67,50,52,47,73,54,54,41,57,45,40,42,65,39,72,54,58,48,66,45,46,67,49,53,57,48,49,52,42,43,68,70,56,50,42,64,60,55,66,39,52,56,41,45,62,36,52,54,55,54,53,62,47,28,47,43,56,49,32,56,56,65,68,63,53,65,44,37,45,59,58,71,41,45,46,53,54,39,51,65,69,51,64,60,60,45,51,64,46,56,52,55,66,62,56,54,58,54,47,43,68,63,48,53,46,66,52,53,41,55,48,59,65,48,46,48,56,61,44,54,73,42,45,27,48,55,51,63,59,55,70,60,51,47,45,61,64,53,50,56,64,44,56,61,57,67,57,35,61,58,56,43,53,39,55,63,18,67,59,57,38,50,56,57,56,77,54,41,52,61,54,48,35,58,52,52,45,63,52,40,38,50,56,58,58,59,55,46,45,50,66,47,74,59,53,59,59,53,38,54,56,65,53,52,52,69,60,61,52,54,33,47,56,54,48,60,55,64,67,68,61,71,43,51,38,40,44,47,50,61,45,62,60,53,43,40,46,49,48,57,57,67,51,42,43,56,48,53,58,54,53,51,33,49,38,43,57,49,33,56,52,62,47,41,55,47,51,61,44,79,51,47,55,63,62,61,63,73,64,51,43,79,41,72,48,54,64,43,38,61,55,66,53,55,43,60,59,58,56,59,46,44,65,60,62,60,65,83,60,58,50,53,38,63,66,60,60,62,53,70,34,44,28,25,55,55,60,58,48,54,58,50,43,58,44,48,57,40,61,68,67,54,62,39,42,48,37,62,45,38,67,44,52,60,60,45,52,57,49,66,49,37,34,57,56,46,48,54,59,69,70,57,63,52,62,40,59,52,53,65,49,65,57,38,44,13,65,55,64,65,61,62,49,62,42,57,57,64,62,53,42,68,50,50,47,60,52,66,51,70,65,59,54,59,42,58,64,61,55,55,67,39,63,60,40,78,44,45,41,65,37,52,60,57,48,63,42,41,58,71,62,63,74,56,56,51,62,50,66,61,33,39,64,50,51,52,56,50,34,59,34,32,70,39,48,60,55,61,70,52,56,57,68,59,51,62,65,58,47,54,40,67,49,46,50,53,63,53,59,51,53,57,57,68,49,59,61,61,48,54,54,53,61,52,46,57,42,61,61,53,60,31,44,49,38,54,50,42,50,71,51,66,43,74,51,44,58,60,56,53,55,60,60,45,57,69,53,81,57,46,39,49,62,49,67,52,53,55,56,51,60,67,56,56,63,57,49,58,55,48,52,42,41,57,65,63,59,47,59,63,45,40,44,55,46,40,46,54,54,34,52,65,48,58,64,41,49,37,37,59,36,46,56,54,56,48,66,57,65,51,52,76,42,51,49,64,40,56,54,69,66,48,60,79,57,53,36,69,47,53,65,58,58,37,51,55,54,58,58,60,59,62,55,51,54,65,48,43,55,56,44,51,58,75,43,59,54,63,39,70,41,46,60,75,43,64,72,40,34,32,64,53,58,51,71,40,67,69,44,44,50,51,52,40,56,54,54,69,60,41,79,65,55,54,55,55,47,57,53,37,69,54,57,55,59,80,53,40,54,56,57,54,58,58,43,63,45,58,57,79,59,34,30,61,33,44,58,61,54,56,56,58,68,55,74,51,61,68,45,75,56,67,59,66,58,66,27,73,43,46,57,40,47,65,51,54,63,66,54,55,40,45,43,42,56,70,48,45,61,61,53,76,42,50,63,62,70,58,59,55,36,54,40,47,68,48,46,69,59,47,52,51,35,55,54,42,35,62,43,50,35,49,48,66,16,48,27,64,67,45,31,45,38,56,56,57,55,73,68,31,34,60,29,41,80,43,34,68,58,45,70,64,46,33,47,42,42,68,46,75,50,48,44,77,68,66,61,60,60,76,54,66,48,56,68,49,57,48,43,54,61,50,57,53,60,65,56,51,54,46,41,53,22,54,39,49,57,61,57,48,51,57,64,61,51,51,52,57,54,42,56,59,52,29,48,41,75,60,47,70,67,57,52,48,47,59,52,21,43,45,56,63,52,66,64,62,34,45,57,61,71,48,37,32,46,36,55,63,49,64,48,50,51,47,50,54,67,30,54,73,33,62,72,61,47,60,65,35,44,63,46,61,48,36,58,62,62,68,48,63,68,54,57,68,54,26,60,46,55,47,43,53,66,58,61,47,69,45,57,44,55,52,52,57,52,63,68,59,53,69,45,49,40,37,39,66,41,58,62,75,58,58,49,59,48,43,49,61,41,67,41,60,45,55,47,71,49,53,71,45,57,69,56,33,57,61,64,62,41,32,45,46,72,53,53,49,51,50,46,55,46,59,46,51,47,67,56,37,41,52,35,68,35,55,76,64,47,50,55,52,49,64,50,43,55,55,53,59,54,57,58,48,46,53,33,77,46,54,54,47,60,41,33,55,58,51,52,54,51,57,57,55,43,56,46,52,55,54,35,49,59,57,56,58,44,60,51,70,48,66,62,56,56,51,45,66,40,53,43,66,57,57,40,64,59,54,40,49,33,37,61,43,57,52,47,55,54,48,47,62,53,52,53,49,70,55,59,57,59,52,61,59,45,62,68,35,49,59,44,78,66,58,52,55,46,58,57,52,40,42,46,60,42,63,44,58,48,40,59,59,48,50,56,68,73,34,39,68,51,50,67,61,51,47,44,65,53,50,29,51,57,65,63,59,57,49,43,46,47,55,60,45,38,64,58,47,55,43,55,73,41,31,52,58,51,51,59,50,50,52,52,62,37,64,69,61,65,64,52,56,32,42,49,43,55,65,57,70,37,65,58,63,33,46,54,51,75,68,62,53,68,52,37,46,64,55,49,33,32,63,51,54,34,68,46,50,52,67,61,69,45,33,66,11,54,63,46,57,43,70,51,61,53,65,53,58,52,48,48,69,52,47,44,52,67,49,66,66,61,51,50,62,46,61,60,54,56,61,51,50,51,46,41,39,53,81,54,40,58,61,49,67,49,66,53,68,60,60,56,41,28,25,41,57,47,50,54,69,51,64,30,62,36,59,60,40,59,53,49,46,70,49,49,54,42,42,37,52,62,57,44,49,58,52,45,71,53,66,63,48,50,54,51,59,61,50,53,44,34,50,55,63,67,61,52,53,45,67,52,48,38,61,61,60,66,61,55,56,56,48,50,75,53,64,62,59,58,54,48,43,46,55,53,55,46,55,57,63,49,77,42,53,60,55,60,56,57,73,49,67,54,49,62,65,53,56,62,53,44,44,53,62,44,61,64,47,57,46,47,51,66,69,46,60,38,46,72,52,55,68,49,51,49,41,68,65,55,59,40,56,46,54,38,45,59,61,55,63,46,62,55,40,43,45,39,49,62,47,46,58,49,50,59,65,48,47,54,65,50,49,62,55,52,52,57,57,65,58,51,61,56,40,42,69,45,55,63,35,64,56,45,55,57,58,42,62,56,45,50,51,53,72,46,53,49,61,69,32,52,78,51,54,45,62,57,60,64,28,65,56,49,38,50,53,34,39,45,69,45,52,37,53,77,60,48,69,46,53,37,45,46,48,53,45,64,66,67,58,19,58,64,42,48,51,53,46,49,65,55,70,41,60,54,57,64,36,41,60,51,53,62,54,53,46,70,48,46,48,50,62,57,54,66,59,51,55,44,59,64,48,59,77,60,51,43,58,51,49,46,58,49,52,56,52,53,47,53,60,71,51,55,57,47,61,53,58,45,53,48,63,39,70,45,58,42,55,57,67,40,65,62,69,53,49,58,45,47,37,49,59,56,30,58,51,43,64,37,51,52,58,58,51,57,57,63,61,65,69,63,50,34,42,53,40,53,66,50,35,62,64,47,57,37,62,60,50,47,33,50,44,61,61,47,53,52,47,65,49,60,50,60,52,50,72,43,63,35,63,41,67,51,58,58,63,56,59,41,38,44,70,62,46,55,39,64,37,53,46,49,53,53,53,46,57,50,67,38,59,46,69,69,53,63,72,49,38,47,37,52,71,47,63,41,71,48,59,39,55,53,52,43,57,48,50,71,52,45,46,42,39,35,41,49,44,39,39,56,69,54,40,52,33,23,42,51,44,64,60,63,37,63,52,53,67,28,65,56,21,35,48,44,52,47,41,53,74,45,32,60,63,58,46,51,54,33,50,52,58,51,54,51,65,48,45,77,59,57,58,42,48,53,51,72,75,60,55,55,51,63,58,48,64,33,54,45,55,53,60,31,64,36,52,84,46,55,68,52,60,41,64,59,47,47,48,69,76,55,51,61,58,28,37,40,58,52,61,46,41,60,54,54,64,55,54,61,51,59,56,43,47,42,35,64,53,35,29,38,40,56,53,60,60,72,59,63,59,49,73,58,42,42,53,28,52,65,51,41,61,57,59,79,55,57,54,39,62,56,57,46,71,55,67,54,66,55,34,63,51,52,56,34,75,53,52,69,62,49,52,42,63,37,57,61,61,41,58,39,47,54,78,52,55,60,63,72,68,56,73,65,50,49,58,63,55,48,67,49,69,46,61,48,46,30,58,61,51,43,44,43,64,55,73,60,45,48,50,50,56,66,42,63,73,54,47,58,54,17,51,49,52,61,71,58,41,55,44,52,71,43,58,53,38,48,53,50,32,67,48,33,65,57,49,48,63,72,49,41,62,60,58,50,64,49,49,37,52,62,58,52,70,65,56,36,75,56,53,51,49,45,45,41,47,46,48,36,55,46,70,52,53,59,70,47,51,51,63,62,53,57,88,53,56,33,52,48,59,46,48,51,56,52,66,52,45,30,46,53,54,72,53,43,54,74,51,44,60,55,51,58,49,46,46,72,61,53,59,73,65,32,53,43,45,55,53,38,52,62,39,62,65,47,49,62,38,56,56,44,45,63,41,63,51,58,59,77,54,71,43,49,52,50,56,50,56,53,52,48,67,68,61,62,51,53,66,55,74,62,58,70,71,39,54,42,70,52,38,60,66,52,67,52,57,59,54,54,58,73,70,60,58,56,76,59,63,47,50,56,63,51,63,56,54,46,61,53,43,45,57,63,68,68,69,60,56,67,72,51,63,53,51,63,36,64,45,51,52,50,43,56,38,57,34,40,34,67,49,53,48,52,58,61,69,48,74,49,48,53,57,52,46,72,63,59,77,59,50,71,49,40,42,65,66,56,68,42,57,56,52,51,65,55,40,60,47,59,52,48,54,60,44,45,66,59,55,42,68,70,60,55,72,44,80,53,44,66,55,53,49,60,69,66,56,47,82,52,57,49,59,58,49,52,66,43,71,39,67,60,57,52,60,48,48,54,59,72,53,77,52,60,75,62,46,53,50,58,38,49,46,39,53,65,35,44,31,58,35,62,56,55,50,52,46,61,56,54,68,57,43,36,51,50,59,45,52,48,66,53,49,61,55,60,53,58,65,31,74,52,50,45,38,63,65,47,49,59,49,55,64,38,57,51,53,67,57,64,57,44,41,61,44,70,49,60,61,53,62,60,57,38,36,57,48,46,47,56,73,42,46,44,66,50,66,55,70,62,54,49,58,58,63,54,43,60,46,53,55,64,57,51,53,49,49,34,44,39,51,55,65,75,59,44,49,47,51,35,47,63,46,74,56,40,73,59,55,45,64,54,62,59,41,58,62,55,60,67,58,54,62,42,38,73,37,54,30,40,60,37,44,38,50,42,40,43,66,53,61,51,70,54,61,41,63,37,61,59,59,42,31,47,70,53,57,46,61,47,71,55,54,63,60,52,60,64,65,60,62,47,80,56,33,46,59,56,56,38,59,30,64,55,71,50,63,27,39,61,41,69,44,62,60,55,54,39,59,41,47,55,51,21,49,59,54,59,57,51,60,45,45,30,50,71,66,57,51,62,55,45,73,42,72,61,46,46,44,59,52,66,55,49,45,53,53,48,52,49,49,45,61,61,66,52,65,53,55,62,58,57,66,48,44,58,62,37,71,57,66,51,59,68,44,42,61,50,66,58,62,58,46,35,59,65,71,53,58,60,67,57,61,49,76,59,62,66,61,29,70,57,47,69,53,55,59,58,40,38,62,59,51,73,61,57,48,61,55,38,53,44,60,57,59,66,47,56,61,51,52,55,56,32,43,41,48,63,54,45,55,61,34,69,56,37,68,66,55,33,57,65,35,52,54,54,48,51,65,77,37,51,38,50,48,33,56,18,50,51,36,45,53,55,53,59,45,48,25,53,55,34,52,56,77,62,60,53,56,56,51,77,54,40,64,50,59,67,62,58,52,57,48,57,49,53,62,68,65,46,57,57,60,69,51,62,62,51,73,52,47,38,36,69,60,24,52,47,64,79,40,49,58,57,44,55,48,48,52,55,50,43,42,60,58,36,40,36,22,73,52,43,52,59,44,34,61,39,69,44,43,62,64,60,48,53,63,49,54,47,63,51,44,51,51,57,47,40,43,55,66,22,47,60,44,57,74,64,32,55,63,54,58,47,59,63,70,47,62,47,69,47,55,57,47,46,67,60,49,69,71,52,54,58,55,76,68,51,72,58,66,67,69,58,53,52,61,50,67,30,71,40,58,40,57,50,64,38,63,43,66,60,44,40,57,47,56,55,43,48,43,41,41,46,57,46,68,59,37,50,52,63,50,44,41,52,51,58,62,53,66,64,29,33,63,45,31,60,57,59,59,61,53,52,51,46,40,71,55,56,59,56,60,64,30,62,59,54,85,53,58,59,57,61,60,51,60,54,65,53,41,39,70,43,34,35,53,46,56,66,53,66,56,59,52,55,58,62,52,21,77,32,52,56,51,62,65,43,54,62,66,49,57,61,57,38,56,45,60,59,53,61,52,60,46,55,63,45,64,68,56,42,53,55,54,48,65,52,48,37,39,58,39,72,45,47,60,65,48,52,69,35,71,40,46,55,51,52,37,56,45,63,76,54,63,73,44,39,43,37,52,35,50,56,61,52,52,60,57,46,53,54,52,47,61,48,59,43,67,38,79,58,35,40,61,39,59,56,48,68,41,11,69,66,58,40,65,59,49,71,47,67,70,59,45,49,67,54,75,49,53,57,68,57,64,56,40,36,46,47,47,65,48,58,61,64,65,55,58,46,44,48,44,55,46,45,70,50,50,69,50,41,33,45,35,56,54,54,48,42,57,49,50,50,47,62,44,38,60,56,51,67,54,45,55,49,55,49,53,52,48,48,52,43,62,54,62,51,42,47,49,65,48,59,70,63,67,35,68,45,61,57,62,60,49,28,63,42,44,52,62,48,53,63,60,67,82,55,58,47,54,44,47,48,84,68,52,58,59,52,63,50,56,52,58,49,60,53,54,31,74,52,46,65,56,48,62,74,47,58,62,62,49,37,39,42,58,47,52,67,43,56,51,38,38,53,33,63,42,58,58,37,45,46,59,50,70,59,55,54,56,49,45,64,66,76,63,66,43,77,58,46,49,55,53,43,58,61,47,46,49,55,58,58,80,52,72,61,66,51,85,54,56,56,73,59,52,40,58,33,55,46,68,48,60,46,50,66,52,63,50,60,62,57,48,41,60,56,52,44,53,44,56,52,62,48,65,42,60,50,65,49,53,64,57,59,56,61,55,61,54,47,51,50,45,56,48,49,44,42,45,64,45,37,40,48,32,54,46,70,57,48,44,51,58,54,51,39,68,66,42,50,67,38,61,45,41,45,63,38,45,45,46,48,60,61,58,42,63,52,51,35,40,57,44,69,61,59,54,52,67,53,63,49,70,57,69,60,48,26,55,56,44,52,52,59,62,62,54,71,60,53,59,55,50,55,54,65,46,48,46,53,69,64,68,45,53,45,67,58,76,67,50,39,34,49,47,55,48,57,65,60,53,50,60,53,40,68,41,50,61,63,47,40,60,48,46,31,49,60,44,55,42,51,53,52,26,57,56,52,31,61,51,46,60,50,65,67,58,50,62,50,53,66,44,56,38,58,53,38,47,57,58,44,47,43,61,62,62,65,63,55,55,34,64,41,57,58,61,40,34,47,53,38,42,52,58,54,63,74,29,67,52,63,62,48,56,61,29,73,55,50,39,60,44,70,46,38,29,48,65,59,42,57,56,36,56,46,33,67,41,61,54,46,55,52,53,45,67,56,45,41,46,65,50,57,61,69,51,30,44,56,43,73,47,64,61,58,60,47,57,57,57,53,49,29,66,49,51,65,61,38,51,55,50,65,64,45,45,64,49,55,71,61,64,47,45,62,71,70,66,53,53,71,52,29,61,61,51,57,53,53,54,55,64,47,57,59,54,50,85,45,59,68,65,43,55,66,60,59,57,47,61,44,45,44,53,57,59,46,67,60,71,64,65,64,54,52,47,62,65,54,56,58,63,67,51,57,76,29,54,49,43,40,58,38,48,46,57,67,35,33,53,48,39,60,53,55,51,58,50,48,45,46,66,59,58,39,32,37,42,66,46,51,60,38,55,68,68,57,36,42,48,56,57,60,64,57,51,31,70,64,71,56,73,46,55,46,58,37,49,51,68,59,57,43,68,58,47,48,54,50,61,30,54,63,42,57,48,50,44,65,74,58,62,50,51,53,57,53,58,55,44,43,65,46,65,42,57,42,57,48,51,63,60,63,63,63,58,41,48,54,41,48,48,45,63,53,55,62,50,42,52,45,36,54,57,49,47,47,55,62,55,33,57,60,63,66,52,50,62,44,42,57,73,51,68,39,59,52,62,51,33,52,46,54,50,53,50,53,34,58,65,61,56,68,62,53,64,48,58,49,55,62,69,55,46,43,71,52,49,62,44,51,47,48,47,56,68,45,44,48,63,63,48,57,40,57,47,55,58,37,62,48,53,54,42,62,63,59,42,40,50,57,46,53,50,44,68,77,35,47,70,39,58,56,47,62,53,43,60,47,53,56,37,48,44,50,32,61,48,57,62,51,57,54,54,59,62,71,47,42,65,52,57,58,57,43,47,56,48,53,49,59,57,44,43,53,68,61,61,48,63,64,55,71,67,45,56,52,60,43,57,27,50,54,58,57,57,48,57,62,47,60,47,44,69,48,55,60,68,61,55,51,70,64,43,45,63,45,56,59,53,44,69,49,65,63,65,53,63,63,55,43,44,51,41,51,36,68,57,54,55,71,67,44,36,71,47,47,38,51,52,45,51,58,51,51,45,38,52,63,51,56,38,59,59,51,42,35,58,45,38,57,58,55,29,59,50,43,52,43,51,58,56,62,43,67,56,64,49,50,57,52,56,45,65,66,49,61,63,42,72,53,59,54,73,56,56,58,47,46,67,62,43,38,58,70,57,52,74,61,63,37,53,48,64,54,41,53,69,54,69,74,64,44,65,52,56,54,57,60,65,76,57,43,52,57,55,51,76,63,62,48,72,54,75,30,67,71,55,42,60,62,55,49,66,66,64,46,50,57,48,61,54,47,64,41,49,50,66,39,68,61,51,39,49,49,47,47,45,38,57,48,59,71,71,55,46,43,66,35,49,69,62,45,50,55,61,55,72,67,67,52,62,41,83,46,52,58,71,59,49,43,55,50,45,49,57,50,79,42,43,66,61,53,50,54,71,70,38,34,56,41,53,55,54,56,53,64,55,45,62,45,62,60,49,48,55,48,46,44,48,51,60,68,72,52,63,45,54,79,48,63,58,32,51,54,66,60,55,51,56,41,27,73,49,53,54,57,51,40,62,53,66,39,41,51,71,43,53,65,45,45,58,56,51,63,69,37,48,55,63,44,54,39,55,45,45,66,64,58,59,75,68,53,54,50,61,50,56,64,66,64,60,43,65,52,52,38,56,53,67,51,59,52,67,53,56,46,56,57,59,59,61,57,62,55,49,62,70,53,58,54,57,59,66,68,44,30,56,46,38,64,50,71,59,69,66,44,47,57,63,69,47,45,62,39,48,48,58,46,56,48,52,48,51,64,45,44,51,58,65,56,61,56,75,56,52,42,53,63,57,60,47,57,67,36,43,72,74,61,45,49,63,59,57,49,60,54,46,48,59,62,53,62,69,54,68,39,52,41,64,65,72,60,49,51,61,65,40,53,48,53,63,49,57,61,63,60,50,45,67,57,40,50,67,50,50,47,56,38,53,38,57,41,45,63,57,57,45,35,52,54,53,51,48,51,65,67,55,33,55,34,60,63,69,51,42,50,47,59,45,44,63,39,56,57,49,42,64,58,45,43,44,54,68,50,48,60,47,43,70,52,43,61,56,34,48,55,54,65,55,65,29,41,66,56,53,61,40,44,50,55,59,55,62,63,55,62,39,55,55,66,78,53,38,51,57,53,69,66,52,60,58,54,74,54,52,58,66,58,66,55,69,58,64,44,61,49,52,49,53,65,58,46,47,69,73,45,59,50,62,37,54,47,51,57,37,48,55,47,50,59,64,32,57,56,45,52,59,51,44,45,57,57,52,51,32,31,41,66,55,50,57,62,57,51,54,64,69,29,59,44,52,49,32,57,62,44,46,44,44,68,66,55,41,53,49,53,32,48,62,39,52,68,50,47,59,57,49,61,60,42,59,48,66,58,68,52,59,55,59,49,47,63,61,42,47,52,49,59,58,52,55,53,67,79,47,62,52,40,56,45,52,58,55,60,53,49,67,45,69,53,54,55,45,54,55,52,63,34,48,67,55,44,49,51,63,82,52,62,50,44,52,48,44,59,52,33,55,37,25,44,57,64,48,53,69,55,61,60,80,43,39,45,68,63,44,59,54,48,63,68,59,76,66,52,52,64,51,36,61,48,63,46,53,48,56,54,50,69,64,50,61,46,49,48,63,58,42,73,42,34,57,57,39,65,41,63,70,63,56,49,53,68,55,46,71,53,43,43,76,61,55,44,54,57,57,41,55,52,64,41,73,60,55,49,61,61,63,57,57,66,64,67,60,53,59,50,56,73,37,36,45,52,55,41,51,64,45,23,44,54,58,65,72,43,56,56,56,56,69,39,70,72,46,43,57,37,42,57,58,39,66,38,48,61,69,32,52,63,61,62,39,40,64,49,66,44,50,45,72,63,64,61,53,63,61,60,63,66,71,65,44,44,69,54,54,49,57,59,61,66,57,62,56,43,59,35,61,64,57,56,69,53,60,56,55,39,60,37,38,52,57,48,58,57,59,54,61,41,68,58,71,75,67,64,43,42,58,64,63,57,35,62,66,57,39,43,51,69,59,42,58,35,49,74,55,48,43,47,65,43,53,67,63,73,45,50,53,47,52,46,67,57,55,63,39,52,65,58,62,63,66,54,66,49,69,44,67,39,56,62,63,58,59,54,51,38,64,56,68,63,62,51,56,44,71,29,33,56,58,59,68,70,38,60,67,60,50,65,68,40,55,46,66,34,61,55,51,60,69,34,60,60,58,43,63,75,51,34,52,54,55,57,60,63,63,64,49,43,44,58,65,61,58,58,51,55,46,57,46,66,62,42,53,49,42,49,64,45,55,56,51,41,63,49,59,63,39,48,54,70,51,46,48,54,52,51,51,51,54,56,57,57,51,54,57,69,67,44,50,44,60,70,63,55,73,62,60,53,56,34,47,59,72,61,44,38,61,53,52,54,42,55,77,52,53,53,69,57,56,64,51,64,45,70,73,59,54,55,54,50,66,51,51,36,46,47,72,67,20,48,55,57,57,70,51,65,58,62,57,49,55,38,58,68,38,49,44,58,52,42,45,53,66,31,48,52,49,65,53,41,63,49,57,47,55,43,67,49,38,41,63,56,61,43,55,63,74,47,56,70,60,52,49,39,69,56,66,47,73,40,65,58,58,51,66,58,64,45,66,52,63,52,55,62,64,54,39,44,87,57,40,59,61,49,61,47,64,53,75,43,55,56,55,41,23,65,58,41,53,33,59,43,69,53,66,59,55,43,48,51,57,33,42,43,57,48,40,53,54,75,33,52,59,69,60,64,41,61,57,39,60,46,55,58,63,53,45,59,52,67,54,47,48,37,48,53,64,29,57,41,35,47,50,61,38,56,72,49,60,58,55,45,65,59,44,45,68,55,50,51,69,54,65,47,60,56,70,58,56,31,60,53,66,55,45,69,69,60,51,35,59,52,64,53,57,61,46,46,48,67,63,58,61,60,69,38,57,58,77,56,69,37,51,44,68,51,52,64,48,46,53,56,65,57,65,50,65,50,50,48,63,73,66,33,64,57,40,69,60,40,47,66,55,38,55,53,52,52,48,45,34,59,62,36,43,30,52,42,76,48,71,77,46,24,52,65,42,66,53,61,62,47,58,57,67,39,42,61,59,46,37,42,43,48,47,55,54,54,47,53,62,45,67,56,48,56,53,55,54,60,49,27,66,63,52,56,61,59,56,51,60,64,56,47,67,54,59,58,64,49,78,42,58,54,55,68,58,53,59,52,59,40,68,45,44,52,57,52,52,55,56,45,62,66,59,51,58,56,61,73,61,52,58,41,55,63,55,65,66,43,48,44,45,68,53,54,61,42,54,60,57,44,78,35,60,54,64,39,46,58,51,42,56,48,64,63,67,56,53,74,53,22,52,47,48,49,51,53,49,56,60,59,67,51,66,54,67,66,59,51,64,68,56,28,60,66,32,55,65,52,56,51,64,65,68,64,51,48,65,53,49,61,65,62,61,52,61,70,42,65,57,54,58,45,64,40,41,60,66,42,64,74,63,61,64,80,73,58,62,41,54,53,36,53,40,68,56,41,57,53,60,35,44,45,53,55,50,67,55,57,54,50,67,55,57,59,74,31,48,42,60,61,52,56,58,31,64,61,62,53,52,59,53,38,49,46,63,49,49,61,77,63,51,46,47,51,60,50,70,40,45,55,74,53,45,45,61,65,32,60,62,37,56,45,59,68,78,42,54,58,63,50,48,46,62,49,50,61,57,55,58,52,53,63,53,55,48,39,62,38,64,51,68,65,50,60,60,52,57,50,58,47,45,46,50,46,55,39,52,46,48,68,64,31,52,58,54,71,59,55,61,33,42,35,71,52,57,41,43,52,50,54,58,54,61,61,62,53,44,71,71,50,44,59,55,37,52,29,61,53,48,63,61,66,54,62,69,52,62,49,68,51,58,64,63,59,58,39,61,57,58,43,65,42,57,54,35,55,67,60,48,63,58,63,51,63,70,57,53,47,86,61,56,48,70,63,50,56,72,60,56,47,38,63,57,66,62,66,74,72,67,63,61,40,48,52,34,59,59,63,69,48,47,53,67,49,47,53,55,63,48,53,55,53,58,40,67,29,75,44,31,49,58,49,40,60,65,64,54,53,64,72,55,34,65,44,50,65,37,58,50,66,37,60,65,60,63,55,67,75,44,67,71,44,61,64,51,71,37,37,47,70,67,70,56,39,62,61,43,57,75,44,59,66,66,51,42,49,68,25,52,35,66,59,50,53,50,39,64,60,74,56,47,46,59,50,61,59,51,59,53,79,63,51,59,45,55,51,34,42,62,34,53,48,36,60,65,43,45,52,67,51,53,52,46,44,37,62,63,57,67,68,63,54,45,35,56,77,61,45,66,52,55,64,58,48,41,60,48,53,57,58,56,38,65,50,79,64,56,68,66,61,66,47,65,36,43,59,56,53,63,31,67,61,44,44,45,49,57,63,46,65,73,44,57,53,66,68,53,56,77,55,51,40,69,64,58,57,60,49,58,64,67,46,34,53,54,34,44,66,63,61,54,67,79,52,54,55,54,49,55,50,68,63,54,39,47,68,63,44,28,42,50,73,45,47,57,68,53,53,58,50,55,54,39,52,63,66,65,65,55,34,56,61,41,73,68,45,55,60,61,56,51,49,65,55,55,63,62,59,58,64,67,62,48,39,60,54,44,32,58,61,49,39,75,62,48,41,48,55,54,56,51,68,87,52,47,68,62,55,52,71,70,55,54,61,65,49,61,43,51,51,56,52,61,49,60,22,51,47,56,71,53,73,60,65,68,61,51,51,58,53,27,66,58,61,62,58,34,67,66,64,52,32,65,48,52,48,66,48,55,54,76,52,59,66,44,34,59,49,43,65,49,46,34,40,55,74,64,40,60,67,34,71,48,62,65,36,56,58,70,49,55,75,63,64,57,45,67,41,66,52,81,85,58,57,73,55,45,52,63,50,53,56,58,54,56,51,58,62,70,73,55,55,79,50,52,46,73,57,55,64,57,61,66,51,69,56,57,41,60,43,37,73,60,60,59,63,47,43,69,55,47,55,65,58,53,60,61,54,34,55,62,57,46,43,40,60,64,47,48,59,49,44,55,47,60,45,35,52,57,46,62,72,42,38,52,43,58,52,65,50,52,67,55,68,43,51,52,58,48,62,64,71,51,58,73,55,70,46,66,56,62,36,65,61,53,48,70,59,62,49,51,58,56,52,61,60,65,54,66,49,67,66,44,52,75,52,54,27,53,43,65,54,50,41,52,61,60,50,50,50,41,56,37,58,54,56,46,63,63,42,51,61,65,47,41,38,65,47,48,51,36,54,63,61,66,56,53,47,56,45,77,31,54,55,53,56,72,43,43,43,57,46,54,43,40,60,65,58,59,56,58,33,36,44,42,47,35,43,67,44,53,60,61,59,54,62,86,67,61,52,57,51,71,54,55,48,52,32,73,63,44,59,57,73,66,63,57,42,75,60,54,57,55,78,43,51,68,42,62,50,51,61,63,56,49,59,53,49,69,47,40,35,60,43,57,68,60,58,47,76,46,58,50,55,51,52,45,54,46,49,61,47,46,50,54,52,59,45,55,62,53,47,48,54,43,52,54,52,71,53,31,60,41,48,48,37,64,62,51,51,66,64,70,59,56,60,57,59,57,55,49,72,55,53,69,58,64,61,54,55,54,36,62,60,38,68,72,67,59,61,68,63,58,62,47,57,69,52,61,65,62,45,63,58,58,48,44,39,71,43,65,51,53,42,78,51,66,48,70,63,69,75,54,58,62,53,51,51,62,60,60,70,75,60,63,40,60,41,55,59,52,63,62,46,62,61,72,47,59,51,52,65,64,47,70,69,52,64,72,48,72,59,62,52,44,53,65,54,58,64,46,45,63,49,46,55,75,48,69,51,42,47,73,36,42,62,71,55};
