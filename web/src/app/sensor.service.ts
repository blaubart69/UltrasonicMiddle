import { Injectable } from '@angular/core';
import { Observable, ReplaySubject, map } from 'rxjs';
import { retry, shareReplay } from 'rxjs/operators'
import { SensorEntry } from '../app/log/log.component';
import { webSocket } from 'rxjs/webSocket';

@Injectable({
  providedIn: 'root'
})
export class SensorService {

	public sensorValue$ : Observable<SensorEntry>;
	public sensorValues$ : Observable<SensorEntry>;

  	constructor() { 

		var url = new URL('/ws', window.location.href);
		url.protocol = url.protocol.replace('http', 'ws');
		console.log("WS connecting to: " + url.href);

		this.sensorValue$ = 
			webSocket(url.href)
				.pipe(
					retry(), 
					map( (valJson : any) => new SensorEntry(valJson.l, valJson.r) )
				);

		this.sensorValues$ = this.sensorValue$.pipe(
			shareReplay(10)
		)
  	}
}
