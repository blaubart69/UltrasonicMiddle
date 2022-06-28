ng build -c production --output-hashing none --output-path ../mc/data
gzip -k ../mc/data/*.html ../mc/data/*.js ../mc/data/*.css
rm      ../mc/data/*.html ../mc/data/*.js ../mc/data/*.css