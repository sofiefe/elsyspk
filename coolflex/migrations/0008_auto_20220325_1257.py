# Generated by Django 3.1.1 on 2022-03-25 11:57

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('coolflex', '0007_auto_20220324_1110'),
    ]

    operations = [
        migrations.AddField(
            model_name='cooluser',
            name='location',
            field=models.CharField(blank=True, default='N/A', max_length=20, null=True),
        ),
        migrations.AddField(
            model_name='cooluser',
            name='timestamp',
            field=models.CharField(blank=True, default='N/A', max_length=20, null=True),
        ),
    ]
