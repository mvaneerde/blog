# Disney Princess data

Underlying data behind [Disney princesses: an attempt at a complete list](https://matthewvaneerde.wordpress.com/2011/08/28/disney-princesses-an-attempt-at-a-complete-list/)

Data is stored in

* [cast.csv](./Data/cast.csv)
* [characters.csv](./Data/characters.csv)
* [movies.csv](./Data/movies.csv)

To format it as HTML, run a command like

```
.\format-data.ps1 > ${env:userprofile}\Desktop\princesses.html
```

Then you can view the generated .html file in a browser