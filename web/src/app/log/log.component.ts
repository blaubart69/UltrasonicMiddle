import { Component, OnInit } from '@angular/core';
import { map, Observable, scan } from 'rxjs';
import { SensorService } from '../sensor.service';

export class SensorEntry {
  constructor (readonly left: number, readonly right: number) {} 

  get diff() : number {
    return this.left - this.right;
  }
}

@Component({
  selector: 'app-log',
  templateUrl: './log.component.html',
  styleUrls: ['./log.component.css']
})
export class LogComponent implements OnInit {

  public sensorValues$: Observable<Array<SensorEntry>> = new Observable<Array<SensorEntry>>();

  constructor(private sensorService : SensorService) {
  }

  ngOnInit(): void {
    this.sensorValues$ = this.sensorService.sensorValues$
    .pipe(
      scan( 
        (values,value) => [value].concat(values.slice(0,19))
        , new Array<SensorEntry>() 
      )
    )
  }
}
