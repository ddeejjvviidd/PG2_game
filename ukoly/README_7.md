# Transparency, antialiasing

## Task 1: Transparency

Allow the App to correctly draw transparent objects.

- create at least 3 (semi)transparent objects
- use correct, full scale transparency (NOT if(alpha<0.1) {discard;} )

## Task 2: Antialiasing

From previous tasks you are using JSON file to set initial window size. Improve startup settings for your app.

- create setting to enable/disable antialiasing
- create setting to set antialiasing level
- warn user if settings are malformed, or non-sense
  - AA on, but level = 1 or lower
  - level > 8
- implement default: no AA
