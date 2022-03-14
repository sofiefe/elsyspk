from django.urls import path
from . import views
from django.contrib.auth.views import LogoutView
from django.conf import settings

urlpatterns = [
  path("", views.home, name="home"),
  path("frontpage", views.frontpage, name="frontpage"),
  path("klasse/<klassenavn>", views.klasse, name="klasse"),
  path("register", views.register_request, name="register"),
  path("login", views.login_request, name="login"),
  path('logout/', LogoutView.as_view(next_page=settings.LOGOUT_REDIRECT_URL), name='logout'), #https://stackoverflow.com/questions/63445084/valueerror-at-logout-logout-didnt-return-an-httpresponse-object

]