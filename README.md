# [Graphia](https://graphia.app/)

[![Build Status](https://github.com/graphia-app/graphia/workflows/Build/badge.svg)](https://github.com/graphia-app/graphia/actions?query=workflow%3ABuild)
![Release](https://img.shields.io/github/v/release/graphia-app/graphia)
![Size](https://img.shields.io/github/repo-size/graphia-app/graphia)
![Downloads](https://img.shields.io/github/downloads/graphia-app/graphia/total)
![Issues](https://img.shields.io/github/issues/graphia-app/graphia)
![Commits](https://img.shields.io/github/commit-activity/m/graphia-app/graphia)

Graphia is a powerful open source visual analytics application developed to aid the interpretation of large and complex datasets.

[![Graphia: a visual analytics platform for complex data](https://img.youtube.com/vi/YjfthA5DIOk/maxresdefault.jpg)](https://www.youtube.com/watch?v=YjfthA5DIOk)

[User guide](https://graphia.app/userguide.html)

## **Features**

- Support for a variety of input data formats, ranging from raw CSV to GraphML
- Create correlation graphs using algorithms such as the Pearson Correlation Coefficient
- Visualisation of millions of data points and relationships
- Interactive visualisation and layout in 2D or 3D
- Flexible search facilities, based on attribute information
- Fast, tunable network clustering using Louvain or MCL algorithms
- Graph metric algorithms including PageRank, Betweeness and Eccentricity
- Enrichment Analysis
- Filter graph elements based on numeric or string based attribute expressions
- Customisable and simple to use web search
- Easy export and sharing of analysis results

## **Many Data Types from Many Sectors**

- **Biological Sciences** - protein interaction data, transcriptomics, single cell analyses, proteomics, metabolomics, multiparameter flow cytometry, genotyping data, medical imaging data
- **Agritech** - data relating to the performance of animals, crops, farms, etc.
- **Fintech** - any numerical data relating to changing variables over time, e.g. share prices or categorical data relating to the attributes of commercial entities
- **Social Media** - network connections between individuals, companies, etc.
- **Text Mining** - count matrices of words found across many documents
- **Questionnaire** - answers to questions are categorical (yes/no) or continuous (1-10)

## **About**

Graphia is designed and built by a small group working in Edinburgh, UK. We are passionate about graphs, the power of visualisation and creating tools that aid the interpretation of complex data.

Our journey started 20 years ago, as we began to envisage how graphs could help us analyse the relationships between genes and proteins. Over this time we developed multiple tools and refined our approach, eventually arriving where we are today with Graphia. We believe it is the best option for interactively visualising large graphs.

## **Help**

If you have a question about how to do something that isn't explained in the [user guide](https://graphia.app/userguide.html), a suggestion for a feature or just want to discuss Graphia in some way, please use the [discussions](https://github.com/graphia-app/graphia/discussions) section on GitHub. If you believe you're experiencing a bug, please report it in the [issues](https://github.com/graphia-app/graphia/issues) section on GitHub and we will endeavour to solve it. For any other more general queries, email us directly: <info@graphia.app>.

Please help us to help you!

## **Acknowledgements** ##

We would like to thank those who have helped us develop Graphia:

* [Scottish Enterprise](https://www.scottish-enterprise.com/)
* [The Roslin Institute](https://www.roslin.ed.ac.uk/)
* [The University of Edinburgh](https://www.ed.ac.uk/)
* [BBSRC](https://bbsrc.ukri.org/)
* Free code signing provided by [SignPath.io](https://about.signpath.io), certificate by [SignPath Foundation](https://signpath.org)

## Pre-release Builds ##

Pre-release builds are [available](https://graphia.dev/?dir=Latest). Stability is not guaranteed, but any testing undertaken is greatly appreciated.

## Building ##

Graphia uses the [CMake](https://cmake.org/) build system. A full build can be performed using the following command:
```
cmake -B build && cmake --build build --parallel
```
Note however that you will usually also need Qt 6 to be installed and indicate to CMake where it lives:
```
CMAKE_PREFIX_PATH=/example/path/to/Qt/6.5.0/gcc_64/ cmake -B build && cmake --build build --parallel
```
