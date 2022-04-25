from django.contrib import admin
from .models import User, CoolUser, Klasse, DataCoolBox
# Register your models here.

#admin.site.register(User)
admin.site.register(CoolUser)
admin.site.register(Klasse)
admin.site.register(DataCoolBox)