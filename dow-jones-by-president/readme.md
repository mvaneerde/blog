For blog post https://matthewvaneerde.wordpress.com/2017/11/02/some-dow-jones-industrial-average-statistics-by-united-states-president/

# Update Dow Jones statistics
1. Download Dow Jones data from https://stooq.com/q/d/?s=^dji
1. Copy it over dow-jones-industrial-average.csv
1. If necessary, update the list of presidents in presidents.csv
1. From PowerShell, run calculate-statistics.ps1
1. Copy the resulting HTML into the blog post

# Update tweets
1. Download Trump tweet data from http://trumptwitterarchive.com/
1. Save it in trump-twitter-archive.json
1. If necessary, update trump-twitter-exceptions.csv
1. From PowerShell, run extract-tweets.ps1 which writes to tweets.csv
1. Run format-tweets.ps1
1. Copy the resulting HTML into the blog post