# given two strings A and B, of equal length
# each consisting of 0s and 1s
#
# delete some characters from the beginning of A
# and from the end of B
# so that the total number of 0s and 1s remaining is the same
#
# find a way to do this so that the total number of
# characters remaining is as large as possible
#
# e.g. A = 001001, B = 011001
# deleting two characters from the beginning of A works
# 001001 => 1001, 011001 => 011001
#
# efficient algorithm:
# first, reverse A so we're only deleting from the end
# r(A) = 100100
# B = 011001
#
# make a table t[i] for each string
# where t[i] is the excess in 1s in the first i characters of the string
# t(r(A)) = 0 1 0 -1 0 -1 -2
# t(B) = 0 -1 0 1 0 -1 0
#
# for example, t(r(A))[5] is -1
# because the first 5 characters of r(A) 10010 has -1 excess 1s
#
# we want to find the largest i + j such that t(r(A))[i] + t(B)[j] = 0
# the answer is 4 + 6 = 10 because 0 + 0 = 0
#
# but how do we find it?
#
# build indexes into each t() - a hash where the keys are the values of t()
# and the value is the LARGEST index of t() with that value
# h(t(r(A))) = {
#     0 => 4
#     1 => 1
#     -1 => 5
#     -2 => 6
# }
# h(t(B)) = {
#     0 => 6
#     -1 => 5
#     1 => 3
# }
#
# now we iterate over both indexes together
# start with (0, 0) which corresponds to each string being balanced
# there is GUARANTEED to be at least one such solution
# in particular we can remove all of both strings
# h(t(r(A)))[0] = 4
# h(t(B))[0] = 6
# we keep 10 characters: 1001, 011001
#
# now look at (1, -1) which corresponds to A having an excess 1
# and B having an excess 0
# h(t(r(A)))[1] = 1
# h(t(B))[-1] = 5
# we keep 6 characters: 1, 01100
#
# now look at (-1, 1) which corresponds to A having an excess 0
# and B having an excess 1
# h(t(r(A)))[-1] = 5
# h(t(B))[1] = 3
# we keep 8 characters: 10010, 011
#
# now look at (2, -2) which corresponds to A having two excess 1s
# and B having two excess 0s
# h(t(r(A)))[2] - no solution 
# h(t(B))[-2] - no solution
#
# now look at (-2, 2) which corresponds to A having two excess 0s
# and B having two excess 2s
# h(t(r(A)))[-2] - 6
# h(t(B))[2] - no solution
#
# at this point we can stop
# of the three solutions we found, 1001, 011001 is the best