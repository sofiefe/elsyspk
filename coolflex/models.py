import datetime
from django.db import models
from django.contrib.auth.models import User

# Create your models here.

class Klasse(models.Model):
    navn = models.CharField(max_length=2, default=None)
    #grade_number = models.IntegerField(max_length=1, default=1)
    #bokstav = models.CharField(max_length=1, default="A")
    teacher = models.ForeignKey(User, on_delete=models.CASCADE)
    def get_grade(self):
        grade = int(self.navn[0])
        return grade
    
    def get_letter(self):
        letter = self.navn[1]
        return letter

class CoolUser(models.Model):
    first_name = models.CharField(max_length=20, default="CoolUser")
    last_name = models.CharField(max_length=20, default ="")
    klasse = models.ForeignKey(Klasse, null=True, on_delete=models.CASCADE)

class DataCoolBox(models.Model):
    cooluser_id = models.IntegerField(max_length=20)
    box_id = models.IntegerField(max_length=20)
    timestamp = models.DateTimeField()

