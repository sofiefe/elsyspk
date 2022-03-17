import datetime
from django.db import models
from django.contrib.auth.models import User

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

    class Meta:
        ordering = ["last_name"]
    
    def __str__(self):
        return self.first_name + " " + self.last_name

class DataCoolBox(models.Model):
    cooluser_id = models.IntegerField(max_length=20)
    box_id = models.IntegerField(max_length=20)
    timestamp = models.DateTimeField()

