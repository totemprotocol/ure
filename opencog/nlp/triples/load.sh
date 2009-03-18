#! /bin/sh
#
# Load all required scm bits and prieces in order to extract triples
#
# Load the generic scm bits
pushd ../../scm
./load3.sh
popd

# Load the NLP scm bits
pushd ../scm
./load-nlp-3.sh
popd

# Load the preposition dictionary
cat prep-maps.scm | netcat localhost 17003

# Load add rules
cat rules.txt | ./rules-to-implications.pl | netcat localhost 17003

# Load some previously parsed sentences.
cat sample-sentences.scm | netcat localhost 17003

