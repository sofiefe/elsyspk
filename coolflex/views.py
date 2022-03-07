from msilib.schema import ListView
from unicodedata import name
from django.shortcuts import render
from django.template import loader
from django.http import HttpResponse
import random
from .models import CoolUser, Design, User
from django.views.generic import ListView
from django.views.generic.edit import CreateView, UpdateView, DeleteView
from .forms import UserForm

# Create your views here.
def home(request):
    nickname = random.choice(nicknames)
    context = {"name" : nickname }
    return render(request, "coolflex/home.html", context)

nicknames = [
    "bestie", "girly", "queen", "king", "boo", "bud", "buddy", "bro", "broski", "bff", "soulmate", "stinky", "homegirl", "bruh", "fave"
]

class CoolUserList(ListView):
    model = CoolUser
    template = 'coolflex/home.html'

   

class DesignList(ListView):
    model = Design
    template = 'coolflex/design.html'


class UserFormCreate(CreateView):
    model = User
    template_name = "cool/userform_template.html"
    form_class = UserForm