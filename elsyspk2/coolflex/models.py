import datetime
from django.db import models
from django.contrib.auth.models import User
from django.shortcuts import redirect

# Create your models here.

class Klasse(models.Model):
    navn = models.CharField(max_length=2, default=None)
    teacher = models.ForeignKey(User, on_delete=models.CASCADE)
    #funksjoner for å hente info
    def get_grade(self):
        grade = int(self.navn[0])
        return grade
    def get_letter(self):
        letter = self.navn[1]
        return letter
    def __str__(self):
        return self.navn
    
    def get_absolute_url(self): #one paramter self, redirects user to path
        return "klasse"

    class Meta:  #la til Meta-data i Klasse og CoolUser for å kunne sortere på klassenavn og etternavn
        ordering = ["navn"]

class CoolUser(models.Model):
    first_name = models.CharField(max_length=20, default="CoolUser")
    last_name = models.CharField(max_length=20, default ="")
    klasse = models.ForeignKey(Klasse, null=True, related_name="cooluser", on_delete=models.CASCADE)
    teacher = models.ForeignKey(User, blank=True, on_delete=models.CASCADE, related_name="+")
    parent = models.ForeignKey(User, blank=True, on_delete=models.CASCADE, related_name="+")
    status = models.CharField(max_length=3, default="DEF", blank=True, null=True)
    timestamp = models.CharField(max_length=20, default="N/A", blank=True, null=True)
    location = models.CharField(max_length=20, default="N/A", blank=True, null=True)

    class Meta:
        ordering = ["last_name", "first_name"]
    
    def __str__(self):
        return self.first_name + " " + self.last_name
    
    def get_absolute_url(self): #one paramter self, redirects user to path
        return "klasse"
    
    def get_parent(self):
        return self.parent.username
    
    def get_teacher(self):
        return self.teacher.username
    

class DataCoolBox(models.Model):
    cooluser_id = models.IntegerField(max_length=20)
    box_id = models.IntegerField(max_length=20)
    timestamp = models.DateTimeField()

    class Meta:
        ordering = ["timestamp"]

    
    