from django.db import models

# Create your models here.
class CoolUser(models.Model):
    first_name = models.CharField(max_length=20, default="friend")
    coolflex = models.IntegerField(default=1)


class Design(models.Model):
    PATTERN_CHOICE = [
        (0, "Blank"),
        (1, "Stripes")
    ]
    COLOR_CHOICE = [ 
        ("RED", "Red"),
        ("BLU", "Blue"),
        ("GRE", "Green"),
        ("WHI", "White")
    ]
    pattern = models.IntegerField(max_length=1, choices=PATTERN_CHOICE, default=0)
    color_one = models.CharField(max_length=3, choices=PATTERN_CHOICE, default="RED")
    color_two = models.CharField(max_length=3, choices=PATTERN_CHOICE, default="WHI")
    user = models.ForeignKey(CoolUser, on_delete=models.CASCADE)



