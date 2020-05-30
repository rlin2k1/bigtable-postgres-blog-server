#!/bin/bash

cbt createtable posts

cbt createfamily posts postcontent

cbt set posts 0 postcontent:title=TestTitle
cbt set posts 0 postcontent:body=TestBody
cbt set posts 1 postcontent:title=TestTitle1
cbt set posts 1 postcontent:body=TestBody1
cbt set posts 2 postcontent:title=TestTitle2
cbt set posts 2 postcontent:body=TestBody2
cbt set posts 3 postcontent:title=TestTitle3
cbt set posts 3 postcontent:body=TestBody3
cbt set posts 4 postcontent:title=TestTitle4
cbt set posts 4 postcontent:body=TestBody4
