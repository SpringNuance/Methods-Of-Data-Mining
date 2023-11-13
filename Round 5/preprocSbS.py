#!/usr/bin/env python
# coding: utf-8
# author Zhang Jun 2021 (with some later modfications)
# Example how to preprocess raw text data. This is not complete/optimized,
# but you can you use the code as a skeleton for your own program.

# To be able to run this code, make sure you have the packages NLTK and scikit-learn installed.
# If not, you can install them with pip via command "pip3 install scikit-learn nltk"
# Load the required packages that are used in this example.
import re
from nltk.tokenize import word_tokenize
from nltk.stem import PorterStemmer
from nltk.stem.snowball import SnowballStemmer
from nltk.corpus import stopwords
from nltk.probability import FreqDist
from sklearn.feature_extraction.text import TfidfVectorizer
from string import punctuation
import numpy as np


# Load data 
data_path = 'acmdocuments.txt'

reader = open(data_path, 'r', encoding='utf-8')
lines = reader.readlines()

# Extract the text from each line
text = [i.split('\t')[0] for i in lines ] 

# some examples
print('First documents:')
for i in text[:10]:
    print(i)
print()


# Preprocessing

# Step 1: tokenization and lowercasing
tokens_list = [word_tokenize(i) for i in text]

lc_tokens_list = []    
for i in tokens_list: 
    lc_tokens_list.append([token.lower() for token in i]) 

print('After tokenization and lowercasing:')
for i in lc_tokens_list[:10]:
    print(i)
print()

# original number of tokens
uniques = np.unique([tok for doc in lc_tokens_list for tok in doc])
print("Original number of tokens: {}\n".format(len(uniques)))



# Steps 2 and 3: remove stop words and punctuation
stop_words = set(stopwords.words('english'))
print('NLTK stopwords:')
print(stop_words)
print()

# Here we include the punctuation in the stop words set. There are alternative
#ways to remove punctuation.
stop_words.update(punctuation)
stop_words.add("...")

#you can check updated stopwords
#print(stop_words)

filtered_sentence = []    
for i in lc_tokens_list: 
    filtered_sentence.append([token for token in i if token not in stop_words]) 
    
# Numbers are also removed
filtered_sentence = [ ' '.join(i) for i in filtered_sentence ]
filtered_sentence = [ re.sub(r'\d+', '', sentence) for sentence in filtered_sentence ]

# number of tokens
uniques = np.unique([tok for doc in filtered_sentence for tok in doc.split()])
print("Number of tokens after stopword and punctuation removal: {}\n".format(len(uniques)))


print('After removing stop words, punctuation and numbers:')
for i in filtered_sentence[:10]:
    print(i)
print()


# Step 4: stemming
porter = PorterStemmer()
#or snowball stemmer
#stemmer = SnowballStemmer("english",ignore_stopwords=True)
stemmed_tokens_list = []

for i in filtered_sentence:
	stemmed_tokens_list.append([porter.stem(j) for j in i.split()])

# number of tokens
uniques = np.unique([tok for doc in stemmed_tokens_list for tok in doc])
print("Number of tokens after stemming: {}\n".format(len(uniques)))

print('After stemming:')
for i in stemmed_tokens_list[:10]:
	for j in i:
		print(j,end=" ")
	print(" ")



#5. Check most frequent words - candidates to add to the stopword list
listofall = [ item for elem in stemmed_tokens_list for item in elem]

freq = FreqDist(listofall);
wnum=freq.B();
print("\nMost common words (total %d)"%wnum)
print(freq.most_common(100))


#6. Present as tf-idf
cleaned_documents = [ ' '.join(i) for i in stemmed_tokens_list]

tfidf_vectorizer = TfidfVectorizer(smooth_idf=False)
#only tf part:
#tfidf_vectorizer = TfidfVectorizer(use_idf=False)

tfidf_vectorizer.fit(cleaned_documents)
tf_idf_vectors = tfidf_vectorizer.transform(cleaned_documents)

print("\nThe tf-idf values of the first document\n");
feature_names = tfidf_vectorizer.get_feature_names_out()
feature_index = tf_idf_vectors[0,:].nonzero()[1]
tfidf_scores = zip(feature_index, [tf_idf_vectors[0, x] for x in feature_index])
for w, s in [(feature_names[i], s) for (i, s) in tfidf_scores]:
    print(w, s)
